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

#include <string>				// std::string.
#include <iostream>				// std::cout, std::endl.
#include <vector>				// std::vector
#include <cmath>				// pow()
#include <thread>
#include <mutex>
#include <deque>

#define _XOPEN_SOURCE 600

#include <stdlib.h>				// grantpt(), posix_openpt(), ptsname(), unlockpt()
#include <fcntl.h>				// posix_openpt()
#include <unistd.h>				// close(), read(), write()
#include <errno.h>

#include "c_structs.hpp"
#include "c_device.hpp"
#include "c_sim.hpp"
#include "uart.hpp"

// -----------------------------------------------------------------------------------------------------------------------------------------------------
Uart::Uart(Device* parent_device_pointer, std::string device_name, bool monitor_on, std::vector<StateDescriptor> in_pin_default_states) 
: Device(parent_device_pointer, device_name, "uart", {"read", "write", "clk"}, {"data_ready"}, monitor_on, in_pin_default_states, 0)
{
	// Create all the address and data bus inputs and outputs and set their default states.
	Configure(in_pin_default_states);
	
	Build();
	Stabilise();
}

Uart::~Uart() {
#ifdef VERBOSE_DTORS
	std::cout << "Uart dtor for " << GetFullName() << " @ " << this << std::endl;
#endif
	Stop();
}

void Uart::Configure(std::vector<StateDescriptor> in_pin_default_states) {

	std::string data_bus_in_prefix = "d_in_";
	std::string data_bus_out_prefix = "d_out_";
	
	CreateBus(s_data_bus_width, data_bus_in_prefix, Pin::Type::IN, in_pin_default_states);
	CreateBus(s_data_bus_width, data_bus_out_prefix, Pin::Type::OUT, {});

	m_clk_pin_index = GetPinPortIndex("clk");
	m_read_pin_index = GetPinPortIndex("read");
	m_write_pin_index = GetPinPortIndex("write");
	m_data_ready_pin_index = GetPinPortIndex("data_ready");

	m_data_bus_in_indices.resize(s_data_bus_width, 0);
	m_data_bus_out_indices.resize(s_data_bus_width, 0);

	for (int i = 0; i < s_data_bus_width; i++) {
		m_data_bus_in_indices[i] = GetPinPortIndex(data_bus_in_prefix + std::to_string(i));
		m_data_bus_out_indices[i] = GetPinPortIndex(data_bus_out_prefix + std::to_string(i));
	}
}

void Uart::Build() {
	// This device does not contain any components!
	// As there are no conventional Components inside the MagicDevice, if we don't mark all of the 'inner terminals' (pin.drive[1] for in pins
	// and pin.drive[0] for out pins) as 'connected', the end-of-build connections check will get upset.
	MarkInnerTerminalsDisconnected();
	m_top_level_sim_pointer->AddSpecialDevice(this);
}

void Uart::Solve() {
	if ((m_pins[m_clk_pin_index].state_changed) && (!m_pins[m_clk_pin_index].state)) {

		if (m_pins[m_read_pin_index].state) {		// Read byte
			unsigned long data_read = m_ptyManager.rxByte();
			for (size_t i = 0; i < s_data_bus_width; i++) {
				Set(m_data_bus_out_indices[i], ((data_read >> i) & 1ul));
			}
			std::cout << "Byte read..." << std::endl;
			if (!m_ptyManager.rxBytesAvailable()) {
				Set(m_data_ready_pin_index, false);
			}
		} else if (!m_pins[m_read_pin_index].state) {
			// Set all data out pins low (F)
			for (size_t i = 0; i < s_data_bus_width; i++) {
				Set(m_data_bus_out_indices[i], false);
			}
		}
		
		if (m_pins[m_write_pin_index].state) {		// Write byte
			unsigned long data_to_write = 0;
			for (size_t i = 0; i < s_data_bus_width; i++) {
				if (m_pins[m_data_bus_in_indices[i]].state) {
					data_to_write |= (1ul << i);
				}
			}
			std::cout << data_to_write << std::endl;
			m_ptyManager.txByte(static_cast<uint8_t>(data_to_write));
		}
	}
	
	Device::Solve();
}

void Uart::Start() {
	m_ptyManager.start();
}

void Uart::Update() {
	if (!m_ptyManager.isErrorFlagSet()) {
		bool outputChanged = false;

		if (!m_pins[m_data_ready_pin_index].state) {
			if (m_ptyManager.rxBytesAvailable()) {
				std::cout << "Updating..." << std::endl;
				Set(m_data_ready_pin_index, true);
				outputChanged = true;
			}
		}

		if (outputChanged) {
			m_parent_device_pointer->QueueToPropagatePrimary(this);
			SolveBackwardsFromParent();
		}
	} else {
		m_top_level_sim_pointer->LogError("UART unable to open PTY");
	}
}

void Uart::Stop() {
	m_ptyManager.stop();
}

PtyManager::~PtyManager() {
	stop();
}

void PtyManager::start() {
	m_run_threads = true;
	m_pty_error_counter = 0;
	m_pty_error_flag = false;
	m_pty_read_thread = std::thread(&PtyManager::ptyReadRuntime, this);
}

void PtyManager::stop() {
	if (m_run_threads) {
		m_pty_available = false;
		m_run_threads = false;
		close(m_master_fd);
		
		m_pty_read_thread.join();
	}
}

bool PtyManager::rxBytesAvailable() {
	std::unique_lock<std::mutex> lock(m_mutex_rx_buffer);
	return m_rx_buffer.size() > 0;
}

bool PtyManager::isErrorFlagSet() {
	return m_pty_error_flag;
}

uint8_t PtyManager::rxByte() {
	std::unique_lock<std::mutex> lock(m_mutex_rx_buffer);
	uint8_t byte = 0;
	if (m_rx_buffer.size() > 0) {
		byte = m_rx_buffer.front();
		m_rx_buffer.pop_front();
	}
	return byte;
}

void PtyManager::txByte(uint8_t byte) {
	if (m_pty_available) {
		write(m_master_fd, (void*)(&byte), 1);
	}
}

void PtyManager::createPty() {
	std::unique_lock<std::mutex> lock(m_mutex_master_fd);
	int master_fd = posix_openpt(O_RDWR | O_NONBLOCK);
	
	if (master_fd > 0) {
		m_master_fd = master_fd;
		grantpt(m_master_fd);
		unlockpt(m_master_fd);
		m_pty_available = true;
		m_pty_error_counter = 0;
		std::cout << "UART available on PTY created at " << ptsname(m_master_fd) << std::endl << std::endl;
		
	} else {
		m_pty_available = false;
		m_pty_error_counter ++;
		std::cout << "UART unable to create PTY" << std::endl << std::endl;

		if (m_pty_error_counter >= 5) {
			m_pty_error_flag = true;
		}
	}
}

void PtyManager::ptyReadRuntime() {
	char rxbuf[1];
	int c; /* to catch read's return value */
	
	while(m_run_threads) {/* main loop */
		if (m_pty_available) {
			/* read from the master file descriptor */
			c = read(m_master_fd, rxbuf, 1);
			if (c == 1) { /* If c is 1, add the received character to the deque */

				//~/* convert carriage return to '\n\r' */
				//~if (rxbuf[0] == '\r') {
					//~txByte(static_cast<uint8_t>('\n'));
					//~txByte(static_cast<uint8_t>('\r'));
				//~}
				//~else { 
					//~txByte(static_cast<uint8_t>(rxbuf[0]));
				//~}
				
				std::unique_lock<std::mutex> lock(m_mutex_rx_buffer);
				m_rx_buffer.push_back(static_cast<uint8_t>(rxbuf[0]));
				
			} else if (c <= 0) { /* if c is not 1, check errno */
				if (errno != EAGAIN) {	// Connection is closed...
					m_pty_available = false;
					std::cout << "UART PTY closed" << std::endl << std::endl;
				}
				// ...otherwise, no waiting data.
			}
		} else {
			if (!m_pty_error_flag) {
				createPty();
			}
		}
	}
	std::cout << "UART reader thread finished" << std::endl << std::endl;
}
