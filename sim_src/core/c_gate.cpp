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
#include <cstdlib>					// rand()

#include "c_core.h"					// Core simulator functionality
#include "utils.h"
#include "strnatcmp.h"
#include "colors.h"

Gate::Gate(Device* parent_device_pointer, std::string const& gate_name, std::string const& gate_type,
	std::vector<std::string> in_pin_names, bool monitor_on) {
	m_device_flag = false;
	m_name = gate_name;
	m_parent_device_pointer = parent_device_pointer;
	m_top_level_sim_pointer = m_parent_device_pointer->GetTopLevelSimPointer();
	m_CUID = m_top_level_sim_pointer->GetNewCUID();
	m_local_component_index = m_parent_device_pointer->GetNewLocalComponentIndex();
	m_nesting_level = m_parent_device_pointer->GetNestingLevel() + 1;
	m_full_name = m_parent_device_pointer->GetFullName() + ":" + m_name;
	m_parent_device_pointer->CreateChildFlags();
	m_component_type = gate_type;
	m_operator_function_pointer = GetOperatorPointer(m_component_type);
	m_monitor_on = monitor_on;
	// If a not gate is being instantiated, cap the inputs list to the first input.
	if (m_component_type == "not") {
		in_pin_names = {"input"};
	} else {
		int number_of_in_pins = in_pin_names.size();
		if (number_of_in_pins < 2) {
			// Log build error here.		-- Not enough pins defined for this Gate!
			std::string build_error = "Gate " + m_full_name + "(" + m_component_type + ") added with only " + std::to_string(number_of_in_pins) + " in pins specified.";
			m_top_level_sim_pointer->LogBuildError(build_error);
		}
	}
	std::sort(in_pin_names.begin(), in_pin_names.end(), compareNat);
	int new_pin_port_index = 0;
	for (const auto& pin_name : in_pin_names) {
		// Assign random states to Gate inputs.
		bool temp_bool = rand() > (RAND_MAX / 2);
		pin new_in_pin = {pin_name, 1, temp_bool, false, new_pin_port_index, {false, false}};
		m_pins.push_back(new_in_pin);
		new_pin_port_index ++;
	}
	std::string out_pin_name = "output";
	m_out_pin_port_index = new_pin_port_index;
	pin new_out_pin = {out_pin_name, 2, false, false, new_pin_port_index, {false, false}};
	m_pins.push_back(new_out_pin);
}

Gate::~Gate() {
	PurgeComponent();
	if (mg_verbose_destructor_flag) {
		std::cout << "Gate dtor for " << m_full_name << " @ " << this << std::endl;
	}
}

void Gate::Reset() {
	// If initially called on this Gate, call it from the top-level Simulation so everything is reset.
	if (!m_top_level_sim_pointer->GetSearchingFlag()) {
		m_top_level_sim_pointer->SetSearchingFlag(true);
		m_top_level_sim_pointer->Reset();
	} else {
		for (auto& this_pin : m_pins) {
			if (this_pin.direction == 1) {
				bool temp_bool = rand() > (RAND_MAX / 2);
				this_pin.state = temp_bool;
			} else {
				this_pin.state = false;
			}
			this_pin.state_changed = false;
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
	bool new_state = (this->*m_operator_function_pointer)(m_pins);
	m_pins[m_out_pin_port_index].state = new_state;
	m_pins[m_out_pin_port_index].state_changed = true;
	m_parent_device_pointer->AppendChildPropagationIdentifier(m_local_component_index);
}

void Gate::Connect(std::vector<std::string> connection_parameters) {	
	if ((connection_parameters.size() == 1) || (connection_parameters.size() == 2)) {
		std::string target_component_name = connection_parameters[0];
		std::string target_pin_name;
		if (connection_parameters.size() == 1) {
			target_pin_name = "input";
		} else {
			target_pin_name = connection_parameters[1];
		}
		Component* target_component_pointer;
		bool target_component_exists = true;
		if (target_component_name == "parent") {
			target_component_pointer = m_parent_device_pointer;
		} else {
			target_component_pointer = m_parent_device_pointer->GetChildComponentPointer(target_component_name);
			if (target_component_pointer == 0) {
				target_component_exists = false;
			}
		}
		if (target_component_exists) {
			bool target_pin_exists = target_component_pointer->CheckIfPinExists(target_pin_name);
			if (target_pin_exists) {
				connection_descriptor new_connection_descriptor;
				new_connection_descriptor.target_component_pointer = target_component_pointer;
				new_connection_descriptor.target_pin_port_index = target_component_pointer->GetPinPortIndex(target_pin_name);
				bool no_existing_connection = true;
				for (const auto& this_connection_descriptor : m_connections) {
					if ((this_connection_descriptor.target_pin_port_index == new_connection_descriptor.target_pin_port_index) && (this_connection_descriptor.target_component_pointer == new_connection_descriptor.target_component_pointer)) {
						no_existing_connection = false;
						break;
					}
				}
				if (no_existing_connection) {
					std::vector<bool> target_pin_already_driven = target_component_pointer->CheckIfPinDriven(new_connection_descriptor.target_pin_port_index);
					if (!target_pin_already_driven[0]) {
						m_connections.push_back(new_connection_descriptor);
						target_component_pointer->SetPinDrivenFlag(new_connection_descriptor.target_pin_port_index, false, true);
						m_pins[m_out_pin_port_index].drive[1] = true;
					} else {
						// Log build error here.		-- This target pin is already driven by another pin.
						std::string build_error = "Gate " + m_full_name + " tried to connect to " + target_component_name + " pin " + target_pin_name + " but it is already driven by another pin.";
						m_top_level_sim_pointer->LogBuildError(build_error);
					}
				} else {
					// Log build error here.		-- This connection already exists.
					std::string build_error = "Gate " + m_full_name + " tried to connect to " + target_component_name + " pin " + target_pin_name + " but is already connected to it.";
					m_top_level_sim_pointer->LogBuildError(build_error);
				}
			} else {
				// Log build error here.		-- Target pin does not exist.
				std::string build_error = "Gate " + m_full_name + " tried to connect to " + target_component_name + " pin " + target_pin_name + " but it does not exist.";
				m_top_level_sim_pointer->LogBuildError(build_error);
			}
		} else {
			// Log build error here.		-- Component does not exist.
			std::string build_error = "Gate " + m_full_name + " tried to connect to " + target_component_name + " but it does not exist.";
			m_top_level_sim_pointer->LogBuildError(build_error);
		}
	} else {
		// Log build error here.		-- Wrong number of connection parameters.
		std::string build_error = "Gate " + m_full_name + " tried to form a connection but the wrong number of connection parameters were provided.";
		m_top_level_sim_pointer->LogBuildError(build_error);
	}
}

void Gate::Set(int pin_port_index, bool state_to_set) {
	pin* this_pin = &m_pins[pin_port_index];
	if (this_pin->state != state_to_set) {
		if (mg_verbose_output_flag) {
			std::cout << BOLD(FBLU("  ->")) << " Gate " << BOLD("" << m_full_name << "") << " terminal " << BOLD("" << this_pin->name << "") << " set from " << BoolToChar(this_pin->state) << " to " << BoolToChar(state_to_set);
		}
		this_pin->state = state_to_set;
		Evaluate();
	}
}

void Gate::Evaluate() {
	// The 'this' below is how we call a method function via it's method function pointer. Assuming the pointer and method
	// are public, we could call it from outside this object via the syntax:
	// 		([object variable name].*[object variable name].[member pointer variable name])(arguments);
	// HOWEVER, from inside the object we use the syntax:
	//		(this->*[member pointer variable name])(arguments);
	bool new_state = (this->*m_operator_function_pointer)(m_pins);
	pin* out_pin = &m_pins[m_out_pin_port_index];
	if (out_pin->state != new_state) {
		if (mg_verbose_output_flag) {
			std::cout << ". Output " << BOLD(FBLU("-> ")) << BoolToChar(new_state) << std::endl;
		}
		// If the gate output has changed add it to the parent Devices propagate_next list, UNLESS this gate
		// is already queued-up to propagate this tick.
		out_pin->state = new_state;
		out_pin->state_changed = true;
		m_parent_device_pointer->QueueToPropagate(m_local_component_index);
		// Print output pin changes if we are monitoring this gate.
		if (m_monitor_on) {
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

void Gate::Propagate() {
	pin* out_pin = &m_pins[m_out_pin_port_index];
	if (out_pin->state_changed) {
		if (mg_verbose_output_flag) {
			std::cout << BOLD(FRED("->")) << "Gate " << BOLD("" << m_full_name << "") << " propagating output = " << BoolToChar(out_pin->state) << std::endl;
		}
		out_pin->state_changed = false;
		for (const auto& this_connection_descriptor : m_connections) {
			this_connection_descriptor.target_component_pointer->Set(this_connection_descriptor.target_pin_port_index, out_pin->state);
		}
	}
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

bool Gate::OperatorAnd(std::vector<pin> const& pins) {
	bool output = true;
	for (const auto& this_pin : pins) {
		if (this_pin.direction == 1) {
			output &= this_pin.state;
		}
	}
	return output;
}

bool Gate::OperatorNand(std::vector<pin> const& pins) {
	bool output = true;
	for (const auto& this_pin : pins) {
		if (this_pin.direction == 1) {
			output &= this_pin.state;
		}
	}
	output = !output;
	return output;
}

bool Gate::OperatorOr(std::vector<pin> const& pins) {
	bool output = false;
	for (const auto& this_pin : pins) {
		if (this_pin.direction == 1) {
			output |= this_pin.state;
		}
	}
	return output;
}

bool Gate::OperatorNor(std::vector<pin> const& pins) {
	bool output = false;
	for (const auto& this_pin : pins) {
		if (this_pin.direction == 1) {
			output |= this_pin.state;
		}
	}
	output = !output;
	return output;
}

bool Gate::OperatorNot(std::vector<pin> const& pins) {
	bool output = false;
	for (const auto& this_pin: pins) {
		if (this_pin.direction == 1) {
			output = !this_pin.state;
		}
	}
	return output;
}

void Gate::PrintPinStates(int max_levels) {
	std::cout << m_full_name << ": [";
	for (const auto& in_pin_name: GetSortedInPinNames()) {
		int in_pin_port_index = GetPinPortIndex(in_pin_name);
		std::cout << " " << BoolToChar(m_pins[in_pin_port_index].state) << " ";
	}
	std::cout << "] [ " << BoolToChar(m_pins[m_out_pin_port_index].state) << " ]" << std::endl;
}

void Gate::ReportUnConnectedPins() {
	if (mg_verbose_output_flag) {
		std::cout << "Checking pins for " << m_full_name << " m_local_component_index = " << m_local_component_index << std::endl;
	}
	for (const auto& this_pin : m_pins) {
		if (this_pin.direction == 1) {
			// We don't halt on a build error for un-driven input pins of upper-most level Gates.
			if ((!this_pin.drive[0]) && (m_nesting_level > 1)) {
				// Log undriven Gate in pin.
				std::string build_error = "Gate " + m_full_name + " in pin " + this_pin.name + " is not driven by any Component.";
				m_top_level_sim_pointer->LogBuildError(build_error);
			}
		} else if (this_pin.direction == 2) {
			if (!this_pin.drive[1]) {
				// Log undriving Gate out pin.
				std::string build_error = "Gate " + m_full_name + " out pin " + this_pin.name + " drives no Component.";
				m_top_level_sim_pointer->LogBuildError(build_error);
			}
		}
	}
}

void Gate::PurgeComponent() {
	std::string header;
	if (mg_verbose_destructor_flag) {
		header =  "Purging -> GATE : " + m_full_name + " @ " + PointerToString(static_cast<void*>(this));
		std::cout << GenerateHeader(header) << std::endl;
	}
	if (!(m_parent_device_pointer->GetDeletionFlag())) {
		// First  - Ask parent device to purge all local references to this Gate...
		//			If we are deleting this Component because we are in the process of deleting
		//			it's parent, we do not need to do this.
		m_parent_device_pointer->PurgeChildConnections(this);
	}
	if (!(m_top_level_sim_pointer->GetDeletionFlag())) {
		// Second - Purge component from Simulation Clocks, Probes and probable_components vector.
		//			This will 'automatically' get rid of any Probes associated with the Component
		//			(as otherwise they would target cleared memory).
		//			If we are deleting this component because we are in the process of deleting
		//			the top-level Simulation, we do not need to do this.
		m_top_level_sim_pointer->PurgeComponentFromProbableComponents(this);
		m_top_level_sim_pointer->PurgeComponentFromClocks(this);
		m_top_level_sim_pointer->PurgeComponentFromProbes(this);
		// Third  - Clear component entry from parent device's m_components.
		m_parent_device_pointer->PurgeChildComponentIdentifiers(this);
	}
	if (mg_verbose_destructor_flag) {
		header =  "GATE : " + m_full_name + " @ " + PointerToString(static_cast<void*>(this)) + " -> Purged.";
		std::cout << GenerateHeader(header) << std::endl;
	}
	// - It should now be safe to delete this object -
}

void Gate::PurgeInboundConnections(Component* target_component_pointer) {
	// Loop over outbound connections and remove any that refer to target component.
	std::vector<connection_descriptor> new_connections = {};
	int connections_removed = 0;
	for (const auto& this_connection_descriptor : m_connections) {
		if (this_connection_descriptor.target_component_pointer != target_component_pointer) {
			// Preserve this connection descriptor.
			new_connections.push_back(this_connection_descriptor);
		} else {
			connections_removed ++;
			if (mg_verbose_destructor_flag) {
				std::cout << "Gate " << m_full_name << " removed an out connection to "
					<< this_connection_descriptor.target_component_pointer->GetFullName() << " in pin "
					<< this_connection_descriptor.target_component_pointer->GetPinName(this_connection_descriptor.target_pin_port_index) << std::endl;
			}
		}
	}
	if ((new_connections.size() == 0) && (connections_removed > 0)) {
		if (mg_verbose_destructor_flag) {
			std::cout << "Gate " + m_full_name + " out pin drive out set to false." << std::endl;
		}
		SetPinDrivenFlag(m_out_pin_port_index, 1, false);
	}
	m_connections = new_connections;
}

void Gate::PurgeOutboundConnections() {
	// Loop over onward connection from out pin and reset destination pins drive-in state.
	for (const auto& this_connection_descriptor : m_connections) {
		Component* target_component_pointer = this_connection_descriptor.target_component_pointer;
		int pin_direction = target_component_pointer->GetPinDirection(this_connection_descriptor.target_pin_port_index);
		std::string direction = "";
		if (pin_direction == 1) {
			direction = " in";
		} else if (pin_direction == 2) {
			direction = " out";
		}
		if (mg_verbose_destructor_flag) {
			std::cout << "Component " << target_component_pointer->GetFullName() << direction << " pin "
				<< target_component_pointer->GetPinName(this_connection_descriptor.target_pin_port_index) << " drive in set to false." << std::endl;
		}
		target_component_pointer->SetPinDrivenFlag(this_connection_descriptor.target_pin_port_index, 0, false);
	}
}
