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
#include <cstdlib>					// rand()

#include "c_core.h"					// Core simulator functionality
#include "utils.h"
#include "strnatcmp.h"
#include "colors.h"

Gate::Gate(Device* parent_device_pointer, std::string const& gate_name, std::string const& gate_type, std::vector<std::string> in_pin_names, bool monitor_on) {
	m_device_flag = false;
	m_name = gate_name;
	m_name_hash = std::hash<std::string>{}(m_name);
	m_parent_device_pointer = parent_device_pointer;
	m_top_level_sim_pointer = m_parent_device_pointer->GetTopLevelSimPointer();
	m_CUID = m_top_level_sim_pointer->GetNewCUID();
	m_nesting_level = m_parent_device_pointer->GetNestingLevel() + 1;
	m_full_name = m_parent_device_pointer->GetFullName() + ":" + m_name;
	m_component_type = gate_type;
	m_operator_function_pointer = GetOperatorPointer(m_component_type);
	m_monitor_on = monitor_on;
	// If a not gate is being instantiated, cap the inputs list to the first input.
	if ((m_component_type == "not") && (in_pin_names.size() > 1)) {
		in_pin_names = {in_pin_names[0]};
	}
	m_sorted_in_pin_names = in_pin_names;
	std::sort(m_sorted_in_pin_names.begin(), m_sorted_in_pin_names.end(), compareNat);
	for (const auto& pin_name: m_sorted_in_pin_names) {
		std::size_t pin_name_hash = std::hash<std::string>{}(pin_name);
		m_sorted_in_pin_name_hashes.push_back(pin_name_hash);
		// Assign random states to Gate inputs.
		bool temp_bool = rand() > (RAND_MAX / 2);
		pin new_in_pin = {pin_name, pin_name_hash, 1, temp_bool, false};
		m_in_pins[pin_name_hash] = new_in_pin;
	}
	m_sorted_out_pin_names = {"output"};
	m_sorted_out_pin_name_hashes = {std::hash<std::string>{}(m_sorted_out_pin_names[0])};
	pin new_out_pin = {m_sorted_out_pin_names[0], m_sorted_out_pin_name_hashes[0], 2, false, false};
	m_out_pins[m_sorted_out_pin_name_hashes[0]] = new_out_pin;
}

void Gate::Connect(std::string const& origin_pin_name, std::string const& target_component_name, std::string const& target_pin_name) {
	std::string connection_identifier = target_component_name + ":" + target_pin_name;
	std::size_t connection_identifier_hash = std::hash<std::string>{}(connection_identifier);
	bool connection_exists = IsHashInMapKeys(connection_identifier_hash, m_connections);
	if (connection_exists == false) {
		Component* target_component;
		if (target_component_name == "parent") {
			target_component = m_parent_device_pointer;
		} else {
			target_component = m_parent_device_pointer->GetChildComponentPointer(target_component_name);
		}
		connection_descriptor conn_descriptor;
		conn_descriptor.target_component = target_component;
		std::size_t target_pin_name_hash = std::hash<std::string>{}(target_pin_name);
		conn_descriptor.target_pin_name_hash = target_pin_name_hash;
		m_connections[connection_identifier_hash] = conn_descriptor;
	} else {
		std::cout << "Duplicate connection omitted." << std::endl;
	}
}

void Gate::Set(std::size_t pin_name_hash, bool state_to_set) {
	pin* this_pin = &m_in_pins[pin_name_hash];
	if (this_pin->state != state_to_set) {
		if (mg_verbose_output_flag) {
			std::cout << BOLD(FBLU("  ->")) << " Gate " << BOLD("" << m_full_name << "") << " terminal " << BOLD("" << this_pin->name << "") << " set from " << BoolToChar(this_pin->state) << " to " << BoolToChar(state_to_set);
		}
		this_pin->state = state_to_set;
		Evaluate();
	}
}

void Gate::Evaluate() {
	pin* this_pin = &m_out_pins[m_sorted_out_pin_name_hashes[0]];
	// The 'this' below is how we call a method function via it's method function pointer. Assuming the pointer and method
	// are public, we could call it from outside this object via the syntax:
	// 		([object variable name].*[object variable name].[member pointer variable name])(arguments);
	// HOWEVER, from inside the object we use the syntax:
	//		(this->*[member pointer variable name])(arguments);
	bool new_state = (this->*m_operator_function_pointer)(m_in_pins);
	if (this_pin->state != new_state) {
		if (mg_verbose_output_flag) {
			std::cout << ". Output " << BOLD(FBLU("-> ")) << BoolToChar(new_state) << std::endl;
		}
		// If the gate output has changed add it to the parent Devices propagate_next list, UNLESS this gate
		// is already queued-up to propagate this tick.
		this_pin->state = new_state;
		if (m_parent_device_pointer->CheckIfQueuedToPropagateThisTick(m_name_hash) == false) {
			m_parent_device_pointer->AddToPropagateNextTick(m_name_hash);
		}
		// Print output pin changes if we are monitoring this gate.
		if (m_monitor_on == true) {
			std::cout << BOLD(FRED(" MONITOR: ")) << BOLD("" << m_full_name << ":" << m_component_type << "") << " output set to " << BoolToChar(new_state) << std::endl;
		}
	} else {
		// Other half of conditional, above, appends text followed by newline character to line printed in previous call
		// *OR* we add the newline character here if no text needs to be appended.
		if (mg_verbose_output_flag) {
			std::cout << std::endl;
		}
	}
}

void Gate::Initialise() {
	// If this gate does not have an input connected to a parent device input, it will not have any inputs Set() during the
	// parent device Stabilise() call, and won't necessarily be evaluated during the subsequent Solve() call. This can result in
	// the parent device stabilising with an incorrect internal state and output states, given it's input states. 
	// To ensure that this does not take place, having set the input states of any gate connected to it's inputs, the parent device
	// calls Initialise() for each remaining child gate to ensure it's output state is sensible with respect to it's initial input 
	// states and that if it's output state has changed this change will be propagated during the subsequent Solve() call.
	bool new_state = (this->*m_operator_function_pointer)(m_in_pins);
	m_out_pins[m_sorted_out_pin_name_hashes[0]].state = new_state;
	if (m_parent_device_pointer->CheckIfQueuedToPropagateThisTick(m_name_hash) == false) {
		m_parent_device_pointer->AddToPropagateNextTick(m_name_hash);
	}
}

void Gate::Propagate() {
	pin* this_pin = &m_out_pins[m_sorted_out_pin_name_hashes[0]];
	bool state_to_propagate = this_pin->state;
	if (mg_verbose_output_flag) {
		std::cout << BOLD(FRED("->")) << "Gate " << BOLD("" << m_full_name << "") << " propagating output = " << BoolToChar(state_to_propagate) << std::endl;
	}
	for (const auto& connection: m_connections) {
		connection_descriptor target_connection_descriptor = connection.second;
		Component* target_component = target_connection_descriptor.target_component;
		std::size_t pin_name_hash = target_connection_descriptor.target_pin_name_hash;
		target_component->Set(pin_name_hash, state_to_propagate);
	}
}

void Gate::MakeProbable() {
	m_top_level_sim_pointer->AddToProbableDevices(m_full_name, this);
}

Component* Gate::GetSiblingComponentPointer (std::string const& target_sibling_component_name) {
	Component* target_component_pointer = m_parent_device_pointer->GetChildComponentPointer(target_sibling_component_name);
	return target_component_pointer;
}

void Gate::PrintPinStates(int max_levels) {
	std::cout << m_full_name << ": [";
	for (const auto& in_pin_name: m_sorted_in_pin_name_hashes) {
		std::cout << " " << BoolToChar(m_in_pins[in_pin_name].state) << " ";
	}
	std::cout << "] [ " << BoolToChar(m_out_pins[m_sorted_out_pin_name_hashes[0]].state) << " ]" << std::endl;
}

operator_pointer Gate::GetOperatorPointer(std::string const& operator_name) {
	operator_pointer pointer;
	if (operator_name == "and") {
		pointer = &Gate::OperatorAnd;
	} else if (operator_name == "nand") {
		pointer = &Gate::OperatorNand;
	} else if (operator_name == "or") {
		pointer = &Gate::OperatorOr;
	} else if (operator_name == "nor") {
		pointer = &Gate::OperatorNor;
	} else if (operator_name == "not") {
		pointer = &Gate::OperatorNot;
	}
	return pointer;
}

bool Gate::OperatorAnd(std::unordered_map<std::size_t, pin> const& in_pins) {
	bool output = true;
	for (const auto& in_pin: in_pins) {
		output &= in_pin.second.state;
	}
	return output;
}
bool Gate::OperatorNand(std::unordered_map<std::size_t, pin> const& in_pins) {
	bool output = true;
	for (const auto& in_pin: in_pins) {
		output &= in_pin.second.state;
	}
	output = !output;
	return output;
}
bool Gate::OperatorOr(std::unordered_map<std::size_t, pin> const& in_pins) {
	bool output = false;
	for (const auto& in_pin: in_pins) {
		output |= in_pin.second.state;
	}
	return output;
}
bool Gate::OperatorNor(std::unordered_map<std::size_t, pin> const& in_pins) {
	bool output = false;
	for (const auto& in_pin: in_pins) {
		output |= in_pin.second.state;
	}
	output = !output;
	return output;
}
bool Gate::OperatorNot(std::unordered_map<std::size_t, pin> const& in_pins) {
	bool output = false;
	for (const auto& in_pin: in_pins) {
		output = !in_pin.second.state;
	}
	return output;
}
