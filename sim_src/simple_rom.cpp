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
#include <cmath>					// pow()
#include <fstream>					// std::ifstream

#include "c_core.h"					// Core simulator functionality
#include "simple_rom.h"

// -----------------------------------------------------------------------------------------------------------------------------------------------------
SimpleRom::SimpleRom(Device* parent_device_pointer, std::string device_name, std::string data_filepath, bool monitor_on,std::vector<state_descriptor> in_pin_default_states) 
 : Device(parent_device_pointer, device_name, "rom", {"read", "clk"}, {}, monitor_on, in_pin_default_states, 0) {
	// We call ConfigureMagic() to create the MagicEngine.
	ConfigureMagic(this, data_filepath);
	// Then we create all the address inputs and data outputs and set their default states.
	ConfigureBusses(in_pin_default_states);
	Build();
	Stabilise();
}

SimpleRom::~SimpleRom() {
	if (m_top_level_sim_pointer->mg_verbose_output_flag) {
		std::cout << "SimpleRom dtor for " << m_full_name << " @ " << this << std::endl;
	}
}

void SimpleRom::Build() {
	// This device does not contain any components!
	// Still need to call MakeProbable() if we want to be able to attach logic probes!
	//~MakeProbable();
	// As there are no conventional Components inside the MagicDevice, if we don't mark all of the 'inner terminals' (pin.drive[1] for in pins
	// and pin.drive[0] for out pins) as 'connected', the end-of-build connections check will get upset.
	MarkInnerTerminalsDisconnected();
}

void SimpleRom::ConfigureMagic(Device* parent_device_pointer, std::string data_filepath) {
	m_magic_device_flag = true;
	m_magic_engine_pointer = new SimpleRom_MagicEngine(parent_device_pointer, data_filepath);
	// Create the necessary magic event trigger.
	// ROM will read data on clk transition T -> F, like a flip-flop.
	AddMagicEventTrap("clk", {true, false}, {{"read", true}}, "MEM_READ");
}

void SimpleRom::ConfigureBusses(std::vector<state_descriptor> in_pin_default_states) {
	std::vector<std::string> inputs = static_cast<SimpleRom_MagicEngine*>(m_magic_engine_pointer)->GenerateInputs();
	std::vector<std::string> outputs = static_cast<SimpleRom_MagicEngine*>(m_magic_engine_pointer)->GenerateOutputs();
	CreateInPins(inputs, in_pin_default_states);
	CreateOutPins(outputs);
	static_cast<SimpleRom_MagicEngine*>(m_magic_engine_pointer)->GetPinPortIndices();
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
SimpleRom_MagicEngine::SimpleRom_MagicEngine(Device* parent_device_pointer, std::string data_filepath) : MagicEngine(parent_device_pointer) {
	m_data_filepath = data_filepath;
	m_data = Configure(data_filepath);		// Sets member variables m_address_bus_width and m_data_bus_width.
}

SimpleRom_MagicEngine::~SimpleRom_MagicEngine() {
	// Shut down the MagicEngine (close any open files, etc...).
	ShutDownMagic();
	if (m_top_level_sim_pointer->mg_verbose_output_flag) {
		std::cout << "SimpleRom_MagicEngine dtor for " << m_identifier << " @ " << this << std::endl;
	}
}

std::vector<std::string> SimpleRom_MagicEngine::GenerateOutputs(void) {
	std::vector<std::string> outputs_to_create;
	for (int index = 0; index < m_data_bus_width; index ++) {
		std::string output_identifier = "d_" + std::to_string(index);
		outputs_to_create.push_back(output_identifier);
	}
	return outputs_to_create;
}

std::vector<std::string> SimpleRom_MagicEngine::GenerateInputs(void) {
	std::vector<std::string> inputs_to_create;
	for (int index = 0; index < m_address_bus_width; index ++) {
		std::string input_identifier = "a_" + std::to_string(index);
		inputs_to_create.push_back(input_identifier);
	}
	return inputs_to_create;
}

void SimpleRom_MagicEngine::GetPinPortIndices() {
	for (int index = 0; index < m_address_bus_width; index ++) {
		std::string input_identifier = "a_" + std::to_string(index);
		m_address_bus_pin_port_indices.push_back(m_parent_device_pointer->GetPinPortIndex(input_identifier));
	}
	for (int index = 0; index < m_data_bus_width; index ++) {
		std::string output_identifier = "d_" + std::to_string(index);
		m_data_bus_pin_port_indices.push_back(m_parent_device_pointer->GetPinPortIndex(output_identifier));
	}
}

std::vector<std::vector<bool>> SimpleRom_MagicEngine::Configure(std::string file_path) {
	// Read in data from file...
	std::vector<std::vector<bool>> padded_file_data;
	std::ifstream data_file (file_path);
	std::string file_row;
	int bus_width;
	int row_index = 0;
	if (data_file.is_open()) {
		while (std::getline(data_file, file_row)) {
			if (row_index == 0) {
				bus_width = file_row.length();
			}
			std::vector<bool> row_data;
			for (int index = 0; index < bus_width; index ++) {
				if (file_row.at(index) == '0') {
					row_data.push_back(false);
				} else {
					row_data.push_back(true);
				}
			}
			padded_file_data.push_back(row_data);
			row_index += 1;
		}
		data_file.close();
		m_data_bus_width = bus_width;
		m_address_bus_width = (int)std::ceil(std::log(row_index)/std::log(2));
		// If this address bus width lets us address more data than we have here (eg: data file has 246 rows),
		// then pad the data out with empty rows.
		int max_address = pow(2, m_address_bus_width);
		if (row_index < max_address) {
			std::vector<bool> empty_row;
			for (int i = 0; i < m_data_bus_width; i ++) {
				empty_row.push_back(false);
			}
			while (row_index < max_address) {
				padded_file_data.push_back(empty_row);
				row_index += 1;
			}
		}
	} else {
		std::cout << "Unable to open file" << std::endl;
	}
	return padded_file_data;
}

void SimpleRom_MagicEngine::UpdateMagic(void) {
	// SimpleRom_MagicEngine has nothing to update.
}

void SimpleRom_MagicEngine::ShutDownMagic(void) {
	// SimpleRom_MagicEngine has nothing to shut down.
}

void SimpleRom_MagicEngine::InvokeMagic(std::string const& incantation) {
	if (incantation == "MEM_READ") {
		// Generate address,
		int address = 0;
		int address_pin_index = 0;
		for (const auto& pin_port_index: m_address_bus_pin_port_indices) {
			bool pin_state = m_parent_device_pointer->GetPinState(pin_port_index);
			if (pin_state) {
				address += m_powers_of_2[address_pin_index];
			}
			address_pin_index ++;
		}
		// Load data from member variable m_data.
		std::vector<bool> data_at_address = m_data[address];
		// Convert std::vector<bool> into states and set parent device outputs accordingly.
		int data_pin_index = 0;
		for (const auto& pin_port_index: m_data_bus_pin_port_indices) {
			if (m_parent_device_pointer->GetPinState(pin_port_index) != data_at_address[data_pin_index]) {
				m_parent_device_pointer->Set(pin_port_index, data_at_address[data_pin_index]);
			}
			data_pin_index ++;
		}
	} else {
		std::cout << "The incantation appears to do nothing...." << std::endl;
	}
}

