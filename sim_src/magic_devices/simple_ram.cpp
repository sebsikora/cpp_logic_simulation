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
#include "simple_ram.h"

// -----------------------------------------------------------------------------------------------------------------------------------------------------
SimpleRam::SimpleRam(Device* parent_device_pointer, std::string device_name, int address_bus_width, int data_bus_width, bool monitor_on, std::vector<state_descriptor> in_pin_default_states) 
 : Device(parent_device_pointer, device_name, "ram", {"read", "write", "clk"}, {}, monitor_on, in_pin_default_states, 0) {
	// Create all the address and data bus inputs and outputs and set their default states.
	ConfigureBusses(address_bus_width, data_bus_width, in_pin_default_states);
	// We call ConfigureMagic() to create the MagicEngine.
	ConfigureMagic(this, address_bus_width, data_bus_width);
	Build();
	Stabilise();
}

SimpleRam::~SimpleRam() {
	if (m_top_level_sim_pointer->mg_verbose_destructor_flag) {
		std::cout << "SimpleRam dtor for " << m_full_name << " @ " << this << std::endl;
	}
}

void SimpleRam::Build() {
	// This device does not contain any components!
	// We still need to call MakeProbable() here during Build() if we want to be able to attach logic probes.
	MakeProbable();
	// As there are no conventional Components inside the MagicDevice, if we don't mark all of the 'inner terminals' (pin.drive[1] for in pins
	// and pin.drive[0] for out pins) as 'connected', the end-of-build connections check will get upset.
	MarkInnerTerminalsDisconnected();
}

void SimpleRam::ConfigureMagic(Device* parent_device_pointer, int address_bus_width, int data_bus_width) {
	m_magic_device_flag = true;
	m_magic_engine_pointer = new SimpleRam_MagicEngine(parent_device_pointer, address_bus_width, data_bus_width);
	// Create the necessary magic event triggers.
	AddMagicEventTrap("clk", {false, true}, {{"read", true}, {"write", false}}, 0);
	AddMagicEventTrap("clk", {false, true}, {{"read", false}, {"write", true}}, 1);
}

void SimpleRam::ConfigureBusses(int address_bus_width, int data_bus_width, std::vector<state_descriptor> in_pin_default_states) {
	std::vector<std::string> inputs_to_create;
	for (int index = 0; index < address_bus_width; index ++) {
		std::string input_identifier = "a_" + std::to_string(index);
		inputs_to_create.push_back(input_identifier);
	}
	for (int index = 0; index < data_bus_width; index ++) {
		std::string input_identifier = "d_in_" + std::to_string(index);
		inputs_to_create.push_back(input_identifier);
	}
	std::vector<std::string> outputs_to_create;
	for (int index = 0; index < data_bus_width; index ++) {
		std::string output_identifier = "d_out_" + std::to_string(index);
		outputs_to_create.push_back(output_identifier);
	}
	CreateInPins(inputs_to_create, in_pin_default_states);
	CreateOutPins(outputs_to_create);
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
SimpleRam_MagicEngine::SimpleRam_MagicEngine(Device* parent_device_pointer, int address_bus_width, int data_bus_width) : MagicEngine(parent_device_pointer) {
	m_address_bus_width = address_bus_width;
	m_data_bus_width = data_bus_width;
	int largest;
	if (m_address_bus_width > m_data_bus_width) {
		largest = m_address_bus_width;
	} else {
		largest = m_data_bus_width;
	}
	for (int index = 0; index < largest; index ++) {
		m_powers_of_two.push_back(pow(2, index));
	}
	ZeroMemory(address_bus_width, data_bus_width);
	GetPinPortIndices(address_bus_width, data_bus_width);
}

SimpleRam_MagicEngine::~SimpleRam_MagicEngine() {
	// Shut down the MagicEngine (close any open files, etc...).
	ShutDownMagic();
	if (m_top_level_sim_pointer->mg_verbose_destructor_flag) {
		std::cout << "SimpleRam_MagicEngine dtor for " << m_identifier << " @ " << this << std::endl;
	}
}

void SimpleRam_MagicEngine::UpdateMagic() {
	// Nothing needs to be routinely updated in this device.
}

void SimpleRam_MagicEngine::ShutDownMagic() {
	// Nothing needs to be shutdown in this device.
}

void SimpleRam_MagicEngine::ZeroMemory(int address_bus_width, int data_bus_width) {
	for (int i = 0; i <= (pow(2, address_bus_width)); i ++) {
		m_data.push_back(0);
	}
}

void SimpleRam_MagicEngine::GetPinPortIndices(int address_bus_width, int data_bus_width) {
	for (int index = 0; index < address_bus_width; index ++) {
		std::string input_identifier = "a_" + std::to_string(index);
		m_address_bus_pin_port_indices.push_back(m_parent_device_pointer->GetPinPortIndex(input_identifier));
	}
	for (int index = 0; index < data_bus_width; index ++) {
		std::string input_identifier = "d_in_" + std::to_string(index);
		m_data_in_bus_pin_port_indices.push_back(m_parent_device_pointer->GetPinPortIndex(input_identifier));
	}
	for (int index = 0; index < data_bus_width; index ++) {
		std::string output_identifier = "d_out_" + std::to_string(index);
		m_data_out_bus_pin_port_indices.push_back(m_parent_device_pointer->GetPinPortIndex(output_identifier));
	}
}

void SimpleRam_MagicEngine::InvokeMagic(int incantation) {
	if (incantation == 0) {						// READ
		// Generate address,
		unsigned int address = 0;
		for (int i = 0; i < m_address_bus_width; i ++) {
			if (m_parent_device_pointer->GetPinState(m_address_bus_pin_port_indices[i])) {
				address |= 1UL << i;
			}
		}
		// Convert std::vector<bool> into states and set parent device outputs accordingly.
		size_t data = m_data[address];
		for (int i = 0; i < m_data_bus_width; i ++) {
			m_parent_device_pointer->Set(m_data_out_bus_pin_port_indices[i], ((data >> i) & 1U));
		}
	} else if (incantation == 1) {				// WRITE
		// Generate address,
		unsigned int address = 0;
		for (int i = 0; i < m_address_bus_width; i ++) {
			if (m_parent_device_pointer->GetPinState(m_address_bus_pin_port_indices[i])) {
				address |= 1UL << i;
			}
		}
		// Get parent device data input pin states and set bits at address correspondingly.
		size_t data = 0;
		for (int i = 0; i < m_data_bus_width; i ++) {
			data |= (m_parent_device_pointer->GetPinState(m_data_in_bus_pin_port_indices[i]) << i);
		}
		m_data[address] = data;
	} else {
		std::cout << "The incantation appears to do nothing...." << std::endl;
	}
}
