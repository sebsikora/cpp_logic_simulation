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

int Component::GetLocalComponentIndex() {
	return m_local_component_index;
}

void Component::SetLocalComponentIndex(int new_local_component_index) {
	m_local_component_index = new_local_component_index;
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
		if ((this_pin.direction == 1) || (this_pin.direction == 3)) {
			sorted_in_pin_names.push_back(this_pin.name);
		}
	}
	std::sort(sorted_in_pin_names.begin(), sorted_in_pin_names.end(), compareNat);
	return sorted_in_pin_names;
}

std::vector<std::string> Component::GetSortedOutPinNames() {
	std::vector<std::string> sorted_out_pin_names = {};
	for (const auto& this_pin : m_pins) {
		if ((this_pin.direction == 2) || (this_pin.direction == 4)) {
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

bool Component::CheckIfPinExists(std::string const& target_pin_name) {
	bool pin_exists = false;
	for (const auto& this_pin : m_pins) {
		if (this_pin.name == target_pin_name) {
			pin_exists = true;
			break;
		}
	}
	return pin_exists;
}

std::vector<bool> Component::CheckIfPinDriven(int pin_port_index) {
	return m_pins[pin_port_index].drive;
}

void Component::SetPinDrivenFlag(int pin_port_index, bool drive_mode, bool state_to_set) {
	if (!drive_mode) {
		m_pins[pin_port_index].drive[0] = state_to_set;
	} else {
		m_pins[pin_port_index].drive[1] = state_to_set;
	}
}

void Component::PrintInPinStates() {
	std::cout << m_name << ": [ ";
	for (const auto& in_pin_name: GetSortedInPinNames()) {
		int in_pin_port_index = GetPinPortIndex(in_pin_name);
		if (m_pins[in_pin_port_index].direction == 1) {
			std::cout << m_pins[in_pin_port_index].name;
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
		int out_pin_port_index = GetPinPortIndex(out_pin_name);
		if (m_pins[out_pin_port_index].direction == 2) {
			std::cout << m_pins[out_pin_port_index].name;
			if (m_pins[out_pin_port_index].state) {
				std::cout << ": T ";
			} else {
				std::cout << ": F ";
			}
		}
	}
	std::cout << "]" << std::endl << std::endl;
}
