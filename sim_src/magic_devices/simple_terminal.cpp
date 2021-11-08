/*
	
    This file is part of cpp_logic_simulation, a simple C++ framework for the simulation of digital logic circuits.
    Copyright (C) 2021 Dr Seb N.F. Sikora
    seb.nf.sikora@protonmail.com
	
    cpp_logic_simulation is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    cpp_logic_simulation is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with cpp_logic_simulation.  If not, see <https://www.gnu.org/licenses/>.

*/

#include <string>					// std::string.
#include <iostream>					// std::cout, std::endl.
#include <vector>					// std::vector
#include <unordered_map>			// std::unordered_map
#include <deque>					// std::deque.
#include <cstring>					// strcat().
#include <cstdio>					// remove().

#include <stdio.h>					// perror().
#include <fcntl.h>					// open() and O_RDONLY, O_WRONLY, etc.
#include <unistd.h>					// write(), read(), sleep(), fork(), execl(), close(), unlink().
#include <sys/stat.h>				// mkinfo().
#include <stdlib.h>					// mkinfo(), exit().
#include <sys/wait.h>				// wait().

#include "c_core.h"					// Core simulator functionality
#include "simple_terminal.h"

// -----------------------------------------------------------------------------------------------------------------------------------------------------
SimpleTerminal::SimpleTerminal(Device* parent_device_pointer, std::string device_name, bool monitor_on, std::vector<state_descriptor> in_pin_default_states) 
 : Device(parent_device_pointer, device_name, "terminal", {"read", "write", "clk", "d_in_0", "d_in_1", "d_in_2", "d_in_3", "d_in_4", "d_in_5", "d_in_6", "d_in_7"},
		  {"d_out_0", "d_out_1", "d_out_2", "d_out_3", "d_out_4", "d_out_5", "d_out_6", "d_out_7", "data_waiting"}, monitor_on, in_pin_default_states, 0) {
	ConfigureMagic(this, device_name);
	Build();
	Stabilise();
}

SimpleTerminal::~SimpleTerminal() {
	if (m_top_level_sim_pointer->mg_verbose_destructor_flag) {
		std::cout << "SimpleTerminal dtor for " << m_full_name << " @ " << this << std::endl;
	}
}

void SimpleTerminal::Build() {
	// This device does not contain any components!
	// We still need to call MakeProbable() here during the Build() process if we want to attach logic probes later.
	MakeProbable();
	// As there are no conventional Components inside the MagicDevice, if we don't mark all of the 'inner terminals' (pin.drive[1] for in pins
	// and pin.drive[0] for out pins) as 'connected', the end-of-build connections check will get upset.
	MarkInnerTerminalsDisconnected();
}

void SimpleTerminal::ConfigureMagic(Device* parent_device_pointer, std::string name) {
	m_magic_device_flag = true;
	m_magic_engine_pointer = new SimpleTerminal_MagicEngine(parent_device_pointer, name);
	// Create the necessary magic event triggers.
	AddMagicEventTrap("clk", {false, true}, {{"read", true}}, 0);
	AddMagicEventTrap("clk", {false, true}, {{"read", false}}, 1);
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
SimpleTerminal_MagicEngine::SimpleTerminal_MagicEngine(Device* parent_device_pointer, std::string name) : MagicEngine(parent_device_pointer) {
	m_data_bus_width = 8;
	GetPinPortIndices(m_data_bus_width);
	m_fifo_dat_s_m_ident_string = "/tmp/fifo_dat_sm_" + name;
	m_fifo_dat_m_s_ident_string = "/tmp/fifo_dat_ms_" + name;
	m_fifo_cmd_m_s_ident_string = "/tmp/fifo_cmd_ms_" + name;
	m_fifo_dat_s_m_ident_carray = m_fifo_dat_s_m_ident_string.c_str();
	m_fifo_dat_m_s_ident_carray = m_fifo_dat_m_s_ident_string.c_str();
	m_fifo_cmd_m_s_ident_carray = m_fifo_cmd_m_s_ident_string.c_str();
	
	// If these temporary files already exist, delete them.
	remove(m_fifo_dat_s_m_ident_carray);
	remove(m_fifo_dat_m_s_ident_carray);
	remove(m_fifo_cmd_m_s_ident_carray);
	
	if ((m_client_pid = StartClient(m_fifo_dat_s_m_ident_carray, m_fifo_dat_m_s_ident_carray, m_fifo_cmd_m_s_ident_carray)) < 0) {
		std::cout << "Couldn't start terminal client!" << std::endl;
	} else {
		std::cout << "Terminal client started." << std::endl;
	}
	
	ConfigureFIFO(m_fifo_dat_s_m_ident_carray, m_fifo_dat_m_s_ident_carray, m_fifo_cmd_m_s_ident_carray, &m_fifo_dat_slave_to_master,
					&m_fifo_dat_master_to_slave, &m_fifo_cmd_master_to_slave);
	
	std::string welcome_message = "--- Simple Terminal Client running ---\n\n";
	SendMessage(welcome_message.c_str(), m_fifo_dat_master_to_slave);
	m_end_code = "end";
	m_match_buffer = "";
}

SimpleTerminal_MagicEngine::~SimpleTerminal_MagicEngine() {
	// Shut down the MagicEngine (close any open files, etc...).
	ShutDownMagic();
	if (m_top_level_sim_pointer->mg_verbose_destructor_flag) {
		std::cout << "SimpleTerminal_MagicEngine dtor for " << m_identifier << " @ " << this << std::endl;
	}
}

void SimpleTerminal_MagicEngine::GetPinPortIndices(int data_bus_width) {
	for (int index = 0; index < data_bus_width; index ++) {
		std::string input_identifier = "d_in_" + std::to_string(index);
		m_data_in_bus_pin_port_indices.push_back(m_parent_device_pointer->GetPinPortIndex(input_identifier));
	}
	for (int index = 0; index < data_bus_width; index ++) {
		std::string output_identifier = "d_out_" + std::to_string(index);
		m_data_out_bus_pin_port_indices.push_back(m_parent_device_pointer->GetPinPortIndex(output_identifier));
	}
	m_data_waiting_pin_port_index = m_parent_device_pointer->GetPinPortIndex("data_waiting");
}

pid_t SimpleTerminal_MagicEngine::StartClient(const char *fifo_dat_s_m_identifier, const char *fifo_dat_m_s_identifier, const char *fifo_cmd_m_s_identifier) {
	pid_t process;
    process = fork();
    if (process < 0)
    {
        perror("fork"); // fork() failed.
        return process;
    }
    if (process == 0) {
		// We are in the child process!
        char arg_string[200] = "";
        char arg_0[] = "./simple_terminal_client/terminal_client";
        char space[] = " ";
        std::strcat(arg_string, arg_0);
        std::strcat(arg_string, space);
        std::strcat(arg_string, fifo_dat_s_m_identifier);
        std::strcat(arg_string, space);
        std::strcat(arg_string, fifo_dat_m_s_identifier);
        std::strcat(arg_string, space);
        std::strcat(arg_string, fifo_cmd_m_s_identifier);
        execl("/usr/bin/xterm", "/usr/bin/xterm", "-e", arg_string, (char*) NULL);
        perror("execl"); // No need to check execv() return value. If it returns, you know it failed.
        return -1;
    }
    return process;
}

int SimpleTerminal_MagicEngine::ConfigureFIFO(const char *fifo_dat_s_m_identifier, const char *fifo_dat_m_s_identifier, const char *fifo_cmd_m_s_identifier,
													int *fifo_dat_s_to_m, int *fifo_dat_m_to_s, int *fifo_cmd_m_to_s) {
	// Create FIFO buffers.
	if (mkfifo(fifo_dat_s_m_identifier, S_IFIFO|0666) != 0) {
		std::cout << "Couldn't create the Slave->Master data FIFO for reading!" << std::endl;
		return 1;
	}
	if (mkfifo(fifo_dat_m_s_identifier, S_IFIFO|0666) != 0) {
		std::cout << "Couldn't create the Master->Slave data FIFO for writing!" << std::endl;
		return 2;
	}
	if (mkfifo(fifo_cmd_m_s_identifier, S_IFIFO|0666) != 0) {
		std::cout << "Couldn't create the Master->Slave command FIFO for writing!" << std::endl;
		return 3;
	}
	// Open FIFO buffers for reading/writing.
	if((*fifo_dat_s_to_m = open(fifo_dat_s_m_identifier, O_RDONLY | O_NONBLOCK)) < 0){
		std::cout << "Couldn't open the Slave->Master data FIFO for reading!" << std::endl;
		return 4;
	}
	if((*fifo_dat_m_to_s = open(fifo_dat_m_s_identifier, O_WRONLY)) < 0){
		std::cout << "Couldn't open the Master->Slave data FIFO for writing!" << std::endl;
		return 5;
	}
	if((*fifo_cmd_m_to_s = open(fifo_cmd_m_s_identifier, O_WRONLY)) < 0){
		std::cout << "Couldn't open the Master->Slave command FIFO for writing!" << std::endl;
		return 6;
	}
	return 0;
}

void SimpleTerminal_MagicEngine::UpdateMagic() {
	// Upon update, read any characters waiting in the slave_to_master FIFO buffer into the internal character buffer.
	char data_in_buffer;
	bool chars_read_in_flag = false;
	while (read(m_fifo_dat_slave_to_master, &data_in_buffer, 1) > 0) {
		if (((m_parent_device_pointer->GetMonitorOnFlag()) || (m_parent_device_pointer->mg_verbose_output_flag)) && !chars_read_in_flag) {
			std::cout << std::endl;
			std::cout << "Reading characters from terminal client fifo buffer into SimpleTerminal input buffer..." << std::endl << std::endl;
		}
		chars_read_in_flag = true;
		m_data_in_char_buffer.insert(m_data_in_char_buffer.begin(), data_in_buffer);
		if ((m_parent_device_pointer->GetMonitorOnFlag()) || (m_parent_device_pointer->mg_verbose_output_flag)) {
			std::string character_to_echo = "";
			if (data_in_buffer == '\n') {
				character_to_echo += "\\n";
			} else {
				character_to_echo += data_in_buffer;
			}
			std::cout << "Received " << character_to_echo << " from terminal client." << std::endl;
		}
	}
	if (((m_parent_device_pointer->GetMonitorOnFlag()) || (m_parent_device_pointer->mg_verbose_output_flag)) && chars_read_in_flag) {
		std::cout << std::endl;
		std::cout << "...Completed." << std::endl << std::endl;
	}
	// If we have read in a character and the "data_waiting" pin is de-asserted, assert it.
	if ((m_data_in_waiting_flag == false) && chars_read_in_flag) {
		m_data_in_waiting_flag = true;
		m_parent_device_pointer->Set(m_data_waiting_pin_port_index, true);
	}
}

void SimpleTerminal_MagicEngine::ShutDownMagic() {
	// Send the kill message to the terminal client, wait for it to finish, and then clean up.
	std::string welcome_message = "end\n";
	SendMessage(welcome_message.c_str(), m_fifo_cmd_master_to_slave);
	Finish(m_client_pid, m_fifo_dat_s_m_ident_carray, m_fifo_dat_m_s_ident_carray, m_fifo_cmd_m_s_ident_carray, &m_fifo_dat_slave_to_master,
			&m_fifo_dat_master_to_slave, &m_fifo_cmd_master_to_slave);
	std::cout << "SimpleTerminal_MagicEngine is shut down." << std::endl;
}

void SimpleTerminal_MagicEngine::InvokeMagic(int incantation) {
	if (incantation == 0) {							// READ
		if (m_data_in_waiting_flag == true) {
			// Get and then remove least-recently received character.
			int data = (int)m_data_in_char_buffer.back();
			m_data_in_char_buffer.pop_back();
			int pin_index = 0;
			for (const auto& pin_port_index : m_data_out_bus_pin_port_indices) {
				if ((data & (1 << pin_index)) == (1 << pin_index)) {
					m_parent_device_pointer->Set(pin_port_index, true);
				} else {
					m_parent_device_pointer->Set(pin_port_index, false);
				}
				pin_index ++;
			}
			// If we have emptied the internal character buffer, de-assert the "data_waiting" output pin.
			if (m_data_in_char_buffer.size() == 0) {
				m_data_in_waiting_flag = false;
				m_parent_device_pointer->Set(m_data_waiting_pin_port_index, false);
			}
		} else {
			// If no data waiting, output zero.
			for (const auto& pin_port_index : m_data_out_bus_pin_port_indices) {
				m_parent_device_pointer->Set(pin_port_index, false);
			}
		}
	} else if (incantation == 1) {							// WRITE
		// Generate data byte,
		int data_byte = 0;
		int pin_index = 0;
		for (const auto& pin_port_index : m_data_in_bus_pin_port_indices) {
			bool pin_state = m_parent_device_pointer->GetPinState(pin_port_index);
			if (pin_state) {
				data_byte |= (1 << pin_index);
			}
			pin_index ++;
		}
		// Send data byte to client.
		char character_to_send = (char)data_byte;
		SendCharacter(character_to_send, m_fifo_dat_master_to_slave);
	} else {
		std::cout << "The incantation appears to do nothing...." << std::endl;
	}
}

void SimpleTerminal_MagicEngine::SendMessage(const char* message, int fifo) {
	int message_length = strlen(message);
	write(fifo, message, message_length);
}

void SimpleTerminal_MagicEngine::SendCharacter(char character, int fifo) {
	write(fifo, &character, 1);
}

void SimpleTerminal_MagicEngine::Finish(pid_t client_pid, const char *fifo_dat_s_m_identifier, const char *fifo_dat_m_s_identifier,
											const char *fifo_cmd_m_s_identifier, int *fifo_dat_slave_to_master, int *fifo_dat_master_to_slave,
											int *fifo_cmd_master_to_slave) {
	int status;
	pid_t wait_result;
	wait_result = waitpid(client_pid, &status, 0);  // Parent process waits here for child to terminate.
	std::cout << "Process " << (unsigned long) wait_result << " returned result: " << status << std::endl;
    std::cout << "Terminal client finished." << std::endl;
	// Close and delete FIFO buffers.
	close(*fifo_dat_slave_to_master);
	close(*fifo_dat_master_to_slave);
	close(*fifo_cmd_master_to_slave);
	unlink(fifo_dat_s_m_identifier);
	unlink(fifo_dat_m_s_identifier);
	unlink(fifo_cmd_m_s_identifier);
}

