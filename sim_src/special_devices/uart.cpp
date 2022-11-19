/*
	
    This file is part of cpp_logic_simulation, a simple C++ framework for the simulation of digital logic circuits.
    Copyright (C) 2022 Dr Seb N.F. Sikora
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
#include <thread>
#include <mutex>

#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include "c_structs.hpp"
#include "c_device.hpp"					// Core simulator functionality
#include "uart.hpp"

// -----------------------------------------------------------------------------------------------------------------------------------------------------
Uart::Uart(Device* parent_device_pointer, std::string device_name, bool monitor_on, std::vector<state_descriptor> in_pin_default_states) 
 : Device(parent_device_pointer, device_name, "uart", {"read", "write", "clk"}, {"data_ready"}, monitor_on, in_pin_default_states, 0) {
	// Create all the address and data bus inputs and outputs and set their default states.
	Configure(in_pin_default_states);
	m_thread_pty_run_flag = true;
	m_thread_pty = std::thread(this, &Uart::PtyReadRuntime);
	Build();
	Stabilise();
}

Uart::~Uart() {
#ifdef VERBOSE_DTORS
	std::cout << "Uart dtor for " << GetFullName() << " @ " << this << std::endl;
#endif
	m_run_pty_thread_flag = false;
	close(m_master_fd);
	m_thread_pty.join();
}

void Uart::CreatePty() {
	std::unique_lock<std::mutex> lock(m_mutex_master_fd);
	int master_fd = posix_openpt(O_RDWR);
	if (master_fd > 0) {
		m_master_fd = master_fd;
		grantpt(m_master_fd);
		unlockpt(m_master_fd);
		printf("Slave: %s\n", ptsname(m_master_fd));
		m_pty_available_flag = true;
	} else {
		printf("Unable to create pseudoterminal\n");
		m_pty_available_flag = false;
	}
}

void Uart::PtyReadRuntime() {
	char rxbuf[1];
	char txbuf[3];
	int c; /* to catch read's return value */

	CreatePty();
	while(m_run_pty_thread_flag) {/* main loop */
		/* read from the master file descriptor */
		c = read(m_master_fd, rxbuf, 1);
		if (c == 1) {
			/* convert carriage return to '\n\r' */
			if (rxbuf[0] == '\r') {
				printf("\n\r"); /* on master */
				sprintf(txbuf, "\n\r"); /* on slave */
			}
			else { 
				printf("%c", rxbuf[0]); 
				sprintf(txbuf, "%c", rxbuf[0]);
			}
			//~fflush(stdout);
			write(m_master_fd, txbuf, strlen(txbuf));
		} else { /* if c is not 1, it has disconnected */
			m_pty_available_flag = false;
			printf("Disconnected\n\r");
			if (m_run_pty_thread_flag) {
				CreatePty();
			}
		}
	}
}

void Uart::PtyWriteByte(uint8_t byte) {
	std::unique_lock<std::mutex> lock(m_mutex_master_fd);
	if (m_pty_available_flag) {
		write(m_master_fd, (void*)(&byte), 1);
	}
}

void Uart::Configure(std::vector<state_descriptor> in_pin_default_states) {
	int data_bus_width = 8;

	std::string data_bus_in_prefix = "d_in_";
	std::string data_bus_in_prefix = "d_out_";
	
	CreateBus(data_bus_width, data_bus_in_prefix, pin::pin_type::IN, in_pin_default_states);
	CreateBus(data_bus_width, data_bus_out_prefix, pin::pin_type::OUT, {});

	m_clk_pin_index = GetPinPortIndex("clk");
	m_read_pin_index = GetPinPortIndex("read");
	m_write_pin_index = GetPinPortIndex("write");
	m_data_ready_pin_index = GetPinPortIndex("data_ready");

	m_data_bus_in_indices.resize(data_bus_width, 0);
	m_data_bus_out_indices.resize(data_bus_width, 0);

	for (int i = 0; i < data_bus_width; i++) {
		m_data_bus_in_indices[i] = GetPinPortIndex(data_bus_in_prefix + std::to_string(i));
		m_data_bus_out_indices[i] = GetPinPortIndex(data_bus_out_prefix + std::to_string(i));
	}
}

void Uart::Build() {
	// This device does not contain any components!
	// As there are no conventional Components inside the MagicDevice, if we don't mark all of the 'inner terminals' (pin.drive[1] for in pins
	// and pin.drive[0] for out pins) as 'connected', the end-of-build connections check will get upset.
	MarkInnerTerminalsDisconnected();
}

void Uart::Solve() {

	Device::Solve();
}
