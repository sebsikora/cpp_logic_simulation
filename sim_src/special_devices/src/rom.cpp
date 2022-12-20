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
#include <fstream>					// std::ifstream
#include <vector>					// std::vector
#include <cmath>					// pow()

#include "c_structs.hpp"
#include "c_device.hpp"					// Core simulator functionality
#include "c_sim.hpp"
#include "rom.hpp"

// -----------------------------------------------------------------------------------------------------------------------------------------------------
Rom::Rom(Device* parent_device_pointer, std::string device_name, std::string const& data_file_path, int address_bus_width, int data_bus_width, bool monitor_on, std::vector<StateDescriptor> in_pin_default_states) 
 : Device(parent_device_pointer, device_name, "rom", {"read", "clk"}, {}, monitor_on, in_pin_default_states, 0) {
	// Create all the address and data bus inputs and outputs and set their default states.
	Configure(address_bus_width, data_bus_width, in_pin_default_states);
	LoadData(data_file_path, address_bus_width);
	Build();
	Stabilise();
}

Rom::~Rom() {
#ifdef VERBOSE_DTORS
	std::cout << "Rom dtor for " << GetFullName() << " @ " << this << std::endl;
#endif
}

void Rom::Configure(int address_bus_width, int data_bus_width, std::vector<StateDescriptor> in_pin_default_states) {
	// Zero the data array.
	m_data.resize(pow(2, address_bus_width), 0);

	std::string address_bus_prefix = "a_";
	std::string data_bus_prefix = "d_";
	
	CreateBus(address_bus_width, address_bus_prefix, Pin::Type::IN, in_pin_default_states);
	CreateBus(data_bus_width, data_bus_prefix, Pin::Type::OUT, {});

	m_clk_pin_index = GetPinPortIndex("clk");
	m_read_pin_index = GetPinPortIndex("read");

	m_address_bus_indices.resize(address_bus_width, 0);
	m_data_bus_indices.resize(data_bus_width, 0);

	for (int i = 0; i < address_bus_width; i++) {
		m_address_bus_indices[i] = GetPinPortIndex(address_bus_prefix + std::to_string(i));
	}

	for (int i = 0; i < data_bus_width; i++) {
		m_data_bus_indices[i] = GetPinPortIndex(data_bus_prefix + std::to_string(i));
	}
}

void Rom::LoadData(std::string const& data_file_path, int address_bus_width) {
	std::ifstream data_file (data_file_path);
	std::string file_row;
	int row_index = 0;
	int row_max = pow(2, address_bus_width);
	
	if (data_file.is_open()) {
		while ((std::getline(data_file, file_row)) && (row_index < row_max)) {
			unsigned long value = std::stoul(file_row, nullptr, 0);
			m_data[row_index] = value;
			++row_index;
		}
		data_file.close();
	} else {
		std::string build_error = "Rom " + GetFullName() + " tried to open data file " + data_file_path + " but the file could not be opened.";
		m_top_level_sim_pointer->LogError(build_error);
	}
}

void Rom::Build() {
	// This device does not contain any components!
	// As there are no conventional Components inside the MagicDevice, if we don't mark all of the 'inner terminals' (pin.drive[1] for in pins
	// and pin.drive[0] for out pins) as 'connected', the end-of-build connections check will get upset.
	MarkInnerTerminalsDisconnected();
}

void Rom::Solve() {
	if ((m_pins[m_clk_pin_index].state_changed) && (!m_pins[m_clk_pin_index].state) && (m_pins[m_read_pin_index].state)) {
		// Calculate address
		unsigned long address = 0;
		for (size_t i = 0; i < m_address_bus_indices.size(); i++) {
			if (m_pins[m_address_bus_indices[i]].state) {
				address |= (1ul << i);
			}
		}

		// Read data
		unsigned long data_read = m_data[address];
		for (size_t i = 0; i < m_data_bus_indices.size(); i++) {
			Set(m_data_bus_indices[i], ((data_read >> i) & 1ul));
		}
	}

	if ((!m_pins[m_read_pin_index].state) && (m_pins[m_read_pin_index].state_changed)) {
		// Set all data out pins low (F)
		for (size_t i = 0; i < m_data_bus_indices.size(); i++) {
			Set(m_data_bus_indices[i], false);
		}
	}

	Device::Solve();
}
