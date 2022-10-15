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

#include "c_structs.hpp"
#include "c_device.hpp"					// Core simulator functionality
#include "simple_ram_redux.hpp"

// -----------------------------------------------------------------------------------------------------------------------------------------------------
SimpleRamRedux::SimpleRamRedux(Device* parent_device_pointer, std::string device_name, int address_bus_width, int data_bus_width, bool monitor_on, std::vector<state_descriptor> in_pin_default_states) 
 : Device(parent_device_pointer, device_name, "ram", {"read", "write", "clk"}, {}, monitor_on, in_pin_default_states, 0) {
	// Create all the address and data bus inputs and outputs and set their default states.
	Configure(address_bus_width, data_bus_width, in_pin_default_states);
	Build();
	Stabilise();
}

SimpleRamRedux::~SimpleRamRedux() {
#ifdef VERBOSE_DTORS
	std::cout << "SimpleRamRedux dtor for " << GetFullName() << " @ " << this << std::endl;
#endif
}

void SimpleRamRedux::Configure(int address_bus_width, int data_bus_width, std::vector<state_descriptor> in_pin_default_states) {
	// Zero the data array.
	m_data.resize(pow(2, address_bus_width), 0);

	std::string address_bus_prefix = "a_";
	std::string data_bus_in_prefix = "d_in_";
	std::string data_bus_out_prefix = "d_out_";
	
	CreateBus(address_bus_width, address_bus_prefix, pin::pin_type::IN, in_pin_default_states);
	CreateBus(data_bus_width, data_bus_in_prefix, pin::pin_type::IN, in_pin_default_states);
	CreateBus(data_bus_width, data_bus_out_prefix, pin::pin_type::OUT, {});

	m_clk_pin_index = GetPinPortIndex("clk");
	m_read_pin_index = GetPinPortIndex("read");
	m_write_pin_index = GetPinPortIndex("write");

	m_address_bus_indices.resize(address_bus_width, 0);
	m_data_bus_in_indices.resize(data_bus_width, 0);
	m_data_bus_out_indices.resize(data_bus_width, 0);

	for (int i = 0; i < address_bus_width; i++) {
		m_address_bus_indices[i] = GetPinPortIndex(address_bus_prefix + std::to_string(i));
	}

	for (int i = 0; i < data_bus_width; i++) {
		m_data_bus_in_indices[i] = GetPinPortIndex(data_bus_in_prefix + std::to_string(i));
		m_data_bus_out_indices[i] = GetPinPortIndex(data_bus_out_prefix + std::to_string(i));
	}
}

void SimpleRamRedux::Build() {
	// This device does not contain any components!
	// As there are no conventional Components inside the MagicDevice, if we don't mark all of the 'inner terminals' (pin.drive[1] for in pins
	// and pin.drive[0] for out pins) as 'connected', the end-of-build connections check will get upset.
	MarkInnerTerminalsDisconnected();
}

void SimpleRamRedux::Solve() {
	if ((m_pins[m_clk_pin_index].state_changed) && (!m_pins[m_clk_pin_index].state)) {

		unsigned int address = 0;
		for (size_t i = 0; i < m_address_bus_indices.size(); i++) {
			if (m_pins[m_address_bus_indices[i]].state) {
				address |= (1u << i);
			}
		}

		if ((m_pins[m_read_pin_index].state) && (!m_pins[m_write_pin_index].state)) {
			// READ
			unsigned int data_read = m_data[address];
			for (size_t i = 0; i < m_data_bus_out_indices.size(); i++) {
				//~bool state_to_set = ((data_read >> (i - m_data_bus_out_start_index)) && 1u) ? true : false;
				Set(m_data_bus_out_indices[i], ((data_read >> i) & 1u));
			}
		} else if ((!m_pins[m_read_pin_index].state) && (m_pins[m_write_pin_index].state)) {
			// WRITE
			unsigned int data_to_write = 0;
			for (size_t i = 0; i < m_data_bus_in_indices.size(); i++) {
				if (m_pins[m_data_bus_in_indices[i]].state) {
					data_to_write |= (1u << i);
				}
			}
			m_data[address] = data_to_write;
		}
	}

	Device::Solve();
}
