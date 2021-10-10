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
#include <functional>				// std::hash
#include <algorithm>				// std::sort

#include "c_core.h"					// Core simulator functionality
#include "utils.h"

bool Component::mg_verbose_output_flag;

std::string Component::GetName() {
	return m_name;
}

std::size_t Component::GetNameHash() {
	return m_name_hash;
}

std::string Component::GetFullName() {
	return m_full_name;
}

bool Component::GetDeviceFlag(void) {
	return m_device_flag;
}

std::string Component::GetComponentType() {
	return m_component_type;
}

Simulation* Component::GetTopLevelSimPointer() {
	return m_top_level_sim_pointer;
}

bool Component::GetInPinState(std::size_t pin_name_hash) {
	return m_in_pins[pin_name_hash].state;
}

bool Component::GetOutPinState(std::size_t pin_name_hash) {
	return m_out_pins[pin_name_hash].state;
}

std::vector<std::string> Component::GetInPinNames() {
	return m_sorted_in_pin_names;
}

std::vector<std::string> Component::GetOutPinNames() {
	return m_sorted_out_pin_names;
}

std::vector<std::size_t> Component::GetInPinNameHashes() {
	return m_sorted_in_pin_name_hashes;
}

std::string Component::GetInPinName(std::size_t in_pin_name_hash) {
	std::string in_pin_name = "";
	for (int i = 0; i < m_sorted_in_pin_name_hashes.size(); i ++) {
		if (in_pin_name_hash == m_sorted_in_pin_name_hashes[i]) {
			in_pin_name = m_sorted_in_pin_names[i];
			break;
		}
	}
	return in_pin_name;
}

std::string Component::GetOutPinName(std::size_t out_pin_name_hash) {
	std::string out_pin_name = "";
	for (int i = 0; i < m_sorted_out_pin_name_hashes.size(); i ++) {
		if (out_pin_name_hash == m_sorted_out_pin_name_hashes[i]) {
			out_pin_name = m_sorted_out_pin_names[i];
			break;
		}
	}
	return out_pin_name;
}

std::vector<int> Component::GetPinDirections(std::vector<std::size_t> const& pin_name_hashes) {
	std::vector<int> pin_directions = {};
	for (const auto& name_hash: pin_name_hashes) {
		int direction = 0;
		for (const auto& in_pin: m_in_pins) {
			if (in_pin.first == name_hash) {
				direction = 1;
			}
		}
		for (const auto& out_pin: m_out_pins) {
			if (out_pin.first == name_hash) {
				direction = 2;
			}
		}
		pin_directions.push_back(direction);
	}
	return pin_directions;
}

int Component::GetPinDirection(size_t pin_name_hash) {
	int pin_direction = 0;
	for (const auto& in_pin: m_in_pins) {
		if (in_pin.first == pin_name_hash) {
			pin_direction = 1;
		}
	}
	for (const auto& out_pin: m_out_pins) {
		if (out_pin.first == pin_name_hash) {
			pin_direction = 2;
		}
	}
	return pin_direction;
}

void Component::PrintInPinStates() {
	std::cout << m_name << ": [ ";
	for (const auto& in_pin_name: m_sorted_in_pin_names) {
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
			std::size_t in_pin_name_hash = std::hash<std::string>{}(in_pin_name);
			if (m_in_pins[in_pin_name_hash].state) {
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
	for (const auto& out_pin_name: m_sorted_out_pin_names) {
		std::cout << out_pin_name;
		std::size_t out_pin_name_hash = std::hash<std::string>{}(out_pin_name);
		if (m_out_pins[out_pin_name_hash].state) {
			std::cout << ": T ";
		} else {
			std::cout << ": F ";
		}
	}
	std::cout << "]" << std::endl << std::endl;
}
