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
#include <algorithm>				// std::sort

#include "c_comp.hpp"
#include "c_sim.hpp"
#include "c_device.hpp"

#include "utils.h"
#include "strnatcmp.h"

Component::~Component() {
#ifdef VERBOSE_DTORS
	std::cout << "Component dtor for " << GetFullName() << " @ " << this << std::endl << std::endl;
#endif
}

std::string Component::GetName() {
	return m_name;
}

std::string Component::GetFullName() {
	std::string full_name = "";
	GenerateFullName(full_name);
	return full_name;
}

void Component::GenerateFullName(std::string &workingString) {
	if (workingString != "") {
		workingString = m_name + ":" + workingString;
	} else {
		workingString = m_name;
	}
	if (this != m_top_level_sim_pointer) {
		m_parent_device_pointer->GenerateFullName(workingString);
	}
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

bool Component::GetPinState(const int pin_port_index) {
	return m_pins[pin_port_index].state;
}

std::string Component::GetPinName(int pin_port_index) {
	return m_pins[pin_port_index].name;
}

std::vector<std::string> Component::GetSortedInPinNames() {
	std::vector<std::string> sorted_in_pin_names = {};
	for (const auto& this_pin : m_pins) {
		if ((this_pin.type == Pin::Type::IN) || (this_pin.type == Pin::Type::HIDDEN_IN)) {
			sorted_in_pin_names.push_back(this_pin.name);
		}
	}
	std::sort(sorted_in_pin_names.begin(), sorted_in_pin_names.end(), compareNat);
	return sorted_in_pin_names;
}

std::vector<std::string> Component::GetSortedOutPinNames() {
	std::vector<std::string> sorted_out_pin_names = {};
	for (const auto& this_pin : m_pins) {
		if ((this_pin.type == Pin::Type::OUT) || (this_pin.type == Pin::Type::HIDDEN_OUT)) {
			sorted_out_pin_names.push_back(this_pin.name);
		}
	}
	std::sort(sorted_out_pin_names.begin(), sorted_out_pin_names.end(), compareNat);
	return sorted_out_pin_names;
}

Pin::Type Component::GetPinType(int pin_port_index) {
	return m_pins[pin_port_index].type;
}

Pin::Type Component::GetPinType(std::string const& pin_name) {
	Pin::Type type = Pin::Type::NONE;
	for (const auto& this_pin: m_pins) {
		if (this_pin.name == pin_name) {
			type = this_pin.type;
			break;
		}
	}
	return type;
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

Pin::Driven* Component::CheckIfPinDriven(int pin_port_index) {
	return &m_pins[pin_port_index].driven;
}

void Component::SetPinDrivenFlag(int pin_port_index, Pin::DriveDirection mode, bool state_to_set) {
	if (mode == Pin::DriveDirection::DRIVE_IN) {
		m_pins[pin_port_index].driven.in = state_to_set;
	} else if (mode == Pin::DriveDirection::DRIVE_OUT) {
		m_pins[pin_port_index].driven.out = state_to_set;
	} else {
		// TODO - Report an error here
	}
}

void Component::PrintInPinStates() {
	std::cout << m_name << ": [ ";
	for (const auto& in_pin_name: GetSortedInPinNames()) {
		int in_pin_port_index = GetPinPortIndex(in_pin_name);
		if (m_pins[in_pin_port_index].type == Pin::Type::IN) {
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
		if (m_pins[out_pin_port_index].type == Pin::Type::OUT) {
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

void Component::SolveBackwardsFromParent() {
	m_parent_device_pointer->Solve();
	m_parent_device_pointer->SolveBackwardsFromParent();
}

Component* Component::GetMonitor() {
	return m_monitor;
}
