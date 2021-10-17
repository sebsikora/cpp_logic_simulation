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
#include <algorithm>				// std::sort

#include "c_core.h"					// Core simulator functionality
#include "utils.h"
#include "strnatcmp.h"

bool Component::mg_verbose_output_flag;			// Global Component verbose_output_flag.

std::string Component::GetName() {
	return m_name;
}

std::string Component::GetFullName() {
	return m_full_name;
}

bool Component::GetDeviceFlag() {
	return m_device_flag;
}

std::string Component::GetComponentType() {
	return m_component_type;
}

Simulation* Component::GetTopLevelSimPointer() {
	return m_top_level_sim_pointer;
}

bool Component::GetPinState(int pin_port_index) {
	return m_pins[pin_port_index].state;
}

std::string Component::GetPinName(int pin_port_index) {
	return m_pins[pin_port_index].name;
}

std::vector<std::string> Component::GetSortedInPinNames() {
	std::vector<std::string> sorted_in_pin_names = {};
	for (const auto& this_pin : m_pins) {
		if (this_pin.direction == 1) {
			sorted_in_pin_names.push_back(this_pin.name);
		}
	}
	std::sort(sorted_in_pin_names.begin(), sorted_in_pin_names.end(), compareNat);
	return sorted_in_pin_names;
}

std::vector<std::string> Component::GetSortedOutPinNames() {
	std::vector<std::string> sorted_out_pin_names = {};
	for (const auto& this_pin : m_pins) {
		if (this_pin.direction == 2) {
			sorted_out_pin_names.push_back(this_pin.name);
		}
	}
	std::sort(sorted_out_pin_names.begin(), sorted_out_pin_names.end(), compareNat);
	return sorted_out_pin_names;
}

int Component::GetPinDirection(int pin_port_index) {
	return m_pins[pin_port_index].direction;
}

int Component::GetPinDirection(std::string const& pin_name) {
	int pin_direction = 0;
	for (const auto& this_pin: m_pins) {
		if (this_pin.name == pin_name) {
			pin_direction = this_pin.direction;
			break;
		}
	}
	return pin_direction;
}

int Component::GetPinPortIndex(std::string const& pin_name) {
	int pin_port_index = 0;
	for (const auto& this_pin : m_pins) {
		if (this_pin.name == pin_name) {
			pin_port_index = this_pin.port_index;
			break;
		}
	}
	return pin_port_index;
}

void Component::PrintInPinStates() {
	std::cout << m_name << ": [ ";
	for (const auto& in_pin_name: GetSortedInPinNames()) {
		bool print_this_input = false;
		if (!m_device_flag) {
			// If we are in a Gate, there are no hidden in states.
			print_this_input = true;
		} else {
			// Otherwise, we are in a device, recast 'this' to Device and check key against hidden in states list.
			Device* device_pointer = static_cast<Device*>(this);
			if (!IsStringInVector(in_pin_name, device_pointer->m_hidden_in_pins)) {
				print_this_input = true;
			} else {
				print_this_input = false;
			}
		}
		if (print_this_input) {
			std::cout << in_pin_name;
			int in_pin_port_index = GetPinPortIndex(in_pin_name);
			if (m_pins[in_pin_port_index].state) {
				std::cout << ": T ";
			} else {
				std::cout << ": F ";
			}
		}
	}
	std::cout << "]" << std::endl << std::endl;
}

void Component::PrintOutPinStates() {
	std::cout << m_name << ": [ ";
	for (const auto& out_pin_name: GetSortedOutPinNames()) {
		bool print_this_output = false;
		if (!m_device_flag) {
			// If we are in a Gate, there are no hidden in states.
			print_this_output = true;
		} else {
			// Otherwise, we are in a device, recast 'this' to Device and check key against hidden in states list.
			Device* device_pointer = static_cast<Device*>(this);
			if (!IsStringInVector(out_pin_name, device_pointer->m_hidden_out_pins)) {
				print_this_output = true;
			} else {
				print_this_output = false;
			}
		}
		if (print_this_output) {
			std::cout << out_pin_name;
			int out_pin_port_index = GetPinPortIndex(out_pin_name);
			if (m_pins[out_pin_port_index].state) {
				std::cout << ": T ";
			} else {
				std::cout << ": F ";
			}
		}
	}
	std::cout << "]" << std::endl << std::endl;
}
