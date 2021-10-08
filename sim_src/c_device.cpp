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
#include "strnatcmp.h"
#include "colors.h"

Device::Device(Device* parent_device_pointer, std::string const& device_name, std::string const& device_type, std::vector<std::string> in_pin_names, std::vector<std::string> const& out_pin_names, bool monitor_on, std::unordered_map<std::string, bool> const& in_pin_default_states, int max_propagations) {
	m_device_flag = true;
	m_name = device_name;
	m_name_hash = std::hash<std::string>{}(m_name);	
	m_parent_device_pointer = parent_device_pointer;
	if (m_parent_device_pointer == this) {
		// The instantiating component is only the same as it's parent component when it is the top-level Simulation.
		m_top_level_sim_pointer = static_cast<Simulation*>(m_parent_device_pointer);
		m_CUID = 0;				// Top-level simulation always CUID 0.
		m_nesting_level = 0;
		m_full_name = m_name;
	} else {
		m_top_level_sim_pointer = m_parent_device_pointer->GetTopLevelSimPointer();
		m_CUID = m_top_level_sim_pointer->GetNewCUID();
		m_nesting_level = m_parent_device_pointer->GetNestingLevel() + 1;
		m_full_name = m_parent_device_pointer->GetFullName() + ":" + m_name;
	}
	m_component_type = device_type;
	m_monitor_on = monitor_on;
	if (max_propagations == 0) {
		// If default max_propagations get the value from the top-level simulation.
		m_max_propagations = m_top_level_sim_pointer->GetTopLevelMaxPropagations();
	} else {
		m_max_propagations = max_propagations;
	}
	in_pin_names.insert(in_pin_names.end(), m_hidden_in_pins.begin(), m_hidden_in_pins.end());
	CreateInPins(in_pin_names, in_pin_default_states);
	if (out_pin_names.size() > 0) {
		CreateOutPins(out_pin_names);
	}
	m_magic_device_flag = false;
}

void Device::CreateInPins(std::vector<std::string> const& pin_names, std::unordered_map<std::string, bool> pin_default_states) {
	// Get existing input state keys in case this is being called from within the constructor
	// for a user-defined Device with fixed inputs already created by the base Device constructor.
	std::vector<std::string> existing_in_pin_names;
	for (const auto& existing_pin_name: m_sorted_in_pin_names) {
		existing_in_pin_names.push_back(existing_pin_name);
	}
	// Create new inputs.
	for (const auto& pin_name: pin_names) {
		std::size_t pin_name_hash = std::hash<std::string>{}(pin_name);
		if (!IsStringInVector(pin_name, m_hidden_in_pins)) {
			// If this is a user-defined input, handle as normal.
			m_pin_directions[pin_name_hash] = 1;
			if (IsStringInMapKeys(pin_name, pin_default_states)) {
				// If this input is in the defaults list set accordingly...
				m_in_pin_states[pin_name_hash] = pin_default_states[pin_name];
			} else {
				// ...otherwise set input state to false.
				m_in_pin_states[pin_name_hash] = false;
			}
		} else {
			// If this is a default 'hidden' input, don't include it in the 'available input' keymap.
			// This means the input cannot be Set().
			m_pin_directions[pin_name_hash] = 0;
			if (pin_name == "true") {
				m_in_pin_states[pin_name_hash] = true;
			} else if (pin_name == "false") {
				m_in_pin_states[pin_name_hash] = false;
			}
		}
		m_in_pin_state_changed[pin_name_hash] = false;
	}
	// Concatenate new and existing input state key lists and sort.
	m_sorted_in_pin_names = pin_names;
	m_sorted_in_pin_names.insert(m_sorted_in_pin_names.end(), existing_in_pin_names.begin(), existing_in_pin_names.end());
	std::sort(m_sorted_in_pin_names.begin(), m_sorted_in_pin_names.end(), compareNat);
	// Pre-hash the input state identifiers for later use. This way, we can use the hashed values of the strings
	// directly as unordered_map keys, which is significantly faster than using the strings as keys whereby they
	// need to be hashed for every map access.
	for (const auto& pin_name: m_sorted_in_pin_names) {
		std::size_t pin_name_hash = std::hash<std::string>{}(pin_name);
		m_sorted_in_pin_name_hashes.push_back(pin_name_hash);
	}
}

void Device::CreateOutPins(std::vector<std::string> const& pin_names) {
	// Get existing output keys in case this is being called from within the constructor
	// for a user-defined Device with fixed outputs already created by the base Device constructor.
	std::vector<std::string> existing_out_pin_names;
	for (const auto& existing_pin_name: m_sorted_out_pin_names) {
		existing_out_pin_names.push_back(existing_pin_name);
	}
	// Create new outputs.
	for (const auto& pin_name: pin_names) {
		std::size_t pin_name_hash = std::hash<std::string>{}(pin_name);
		m_out_pin_states[pin_name_hash] = false;
		m_out_pin_state_changed[pin_name_hash] = false;
		m_pin_directions[pin_name_hash] = 2;
	}
	// Concatenate new and existing output state key lists and sort.
	m_sorted_out_pin_names = pin_names;
	m_sorted_out_pin_names.insert(m_sorted_out_pin_names.end(), existing_out_pin_names.begin(), existing_out_pin_names.end());
	std::sort(m_sorted_out_pin_names.begin(), m_sorted_out_pin_names.end(), compareNat);
	// Pre-hash the output state identifiers for later use. This way, we can use the hashed values of the strings
	// directly as unordered_map keys, which is significantly faster than using the strings as keys whereby they
	// need to be hashed for every map access.
	for (const auto& pin_name: m_sorted_out_pin_names) {
		std::size_t pin_name_hash = std::hash<std::string>{}(pin_name);
		m_sorted_out_pin_name_hashes.push_back(pin_name_hash);
	}
}

void Device::AddComponent(Component* new_component_pointer) {
	std::size_t new_component_name_hash = new_component_pointer->GetNameHash();
	m_components[new_component_name_hash] = new_component_pointer;
}

void Device::AddGate(std::string const& component_name, std::string const& component_type, std::vector<std::string> const& in_pin_names, bool monitor_on) {
	AddComponent(new Gate(this, component_name, component_type, in_pin_names, monitor_on));
}

void Device::AddMagicEventTrap(std::string const& target_pin_name, std::vector<bool> const& state_change, std::vector<human_writable_magic_event_co_condition> const& hw_co_conditions, std::string const& incantation) {
	if (m_magic_device_flag == true) {
		// Convert the human-writable  kind of magic event co-condition (strings for terminal identifiers) to the kind with pre-hashed identifiers.
		std::vector<magic_event_co_condition> co_conditions = {};
		for (int i = 0; i  < hw_co_conditions.size(); i ++) {
			magic_event_co_condition this_co_condition;
			this_co_condition.pin_name_hash = std::hash<std::string>{}(hw_co_conditions[i].pin_name);
			this_co_condition.pin_state = hw_co_conditions[i].pin_state;
			co_conditions.push_back(this_co_condition);
		}
		magic_event new_magic_event;
		std::size_t target_pin_name_hash = std::hash<std::string>{}(target_pin_name);
		new_magic_event.target_pin_name_hash = target_pin_name_hash;
		new_magic_event.state_change = state_change;
		new_magic_event.co_conditions = co_conditions;
		new_magic_event.incantation = incantation;
		std::string identifier = target_pin_name + ":" + incantation;
		m_magic_engine_pointer->AddMagicEventTrap(identifier, new_magic_event);
	} else {
		std::cout << "Device " + m_full_name + " is not magic! Cannot add magic event trap." << std::endl;
	}
}

void Device::ChildConnect(std::string const& target_child_component, std::vector<std::string> const& connection_parameters) {
	std::size_t target_child_name_hash = std::hash<std::string>{}(target_child_component);
	Component* target_component = m_components[target_child_name_hash];
	target_component->Connect(connection_parameters[0], connection_parameters[1], connection_parameters[2]);
}

void Device::ChildSet(std::string const& target_child_component_name, std::string const& target_pin_name, bool logical_state) {
	std::size_t target_child_name_hash = std::hash<std::string>{}(target_child_component_name);
	std::size_t target_pin_name_hash = std::hash<std::string>{}(target_pin_name);
	Component* target_component = m_components[target_child_name_hash];
	if (mg_verbose_output_flag || target_component->m_monitor_on) {
		std::cout << BOLD(FYEL("CHILDSET: ")) << "Component " << BOLD("" << target_component->GetFullName() << ":" << target_component->GetComponentType() << "") << " terminal " << BOLD("" << target_pin_name << "") << " set to " << BoolToChar(logical_state) << std::endl;
	}
	target_component->Set(target_pin_name_hash, logical_state);
	if (mg_verbose_output_flag) {
		std::cout << std::endl;
	}
	// If this is a 1st-level device, if the simulation is not running the user would need to call Solve() after every
	// 'manual' pin change to make sure that 1st-level device state is propagated. Instead, we check for them here if
	// the simulation is not running and call Solve() at the end of the Set() call.
	if (!m_top_level_sim_pointer->IsSimulationRunning()) {
		m_top_level_sim_pointer->Solve();
	}
}

void Device::ChildPrintPinStates(std::string const& target_child_component_name, int max_levels) {
	std::size_t target_child_component_name_hash = std::hash<std::string>{}(target_child_component_name);
	Component* target_component = m_components[target_child_component_name_hash];
	target_component->PrintPinStates(max_levels);
}

void Device::ChildPrintInPinStates(std::string const& target_child_component_name) {
	std::size_t target_child_component_name_hash = std::hash<std::string>{}(target_child_component_name);
	Component* target_component = m_components[target_child_component_name_hash];
	target_component->PrintInPinStates();
}

void Device::ChildPrintOutPinStates(std::string const& target_child_component_name) {
	std::size_t target_child_component_name_hash = std::hash<std::string>{}(target_child_component_name);
	Component* target_component = m_components[target_child_component_name_hash];
	target_component->PrintOutPinStates();
}

void Device::ChildMakeProbable(std::string const& target_child_component_name) {
	std::size_t target_child_component_name_hash = std::hash<std::string>{}(target_child_component_name);
	Component* target_component = m_components[target_child_component_name_hash];
	target_component->MakeProbable();
}

void Device::Build() {
	// Redefined for each specific device subclass...
}

void Device::Stabilise() {
	// Ensures that internal device state settles correctly. 
	if (mg_verbose_output_flag) {
		std::string msg = "Stabilising new level " + std::to_string(m_nesting_level) + " device " + m_full_name;
		std::cout << GenerateHeader(msg) << std::endl << std::endl;
	}
	// First we Set() all child component inputs that are connected to the parent device inputs to deafult states.
	for (const auto& pin_name_hash: m_sorted_in_pin_name_hashes) {
		for (const auto& connection: m_ports[pin_name_hash]) {
			connection_descriptor target_connection_descriptor = connection.second;
			Component* target_component = target_connection_descriptor.target_component;
			std::size_t target_pin_name_hash = target_connection_descriptor.target_pin_name_hash;
			target_component->Set(target_pin_name_hash, m_in_pin_states[pin_name_hash]);
		}
	}
	// Next we call Initialise() for all components.
	if (mg_verbose_output_flag) {
		std::cout << std::endl;
	}
	for (const auto& component_identifier: m_components) {
		if (mg_verbose_output_flag) {
			std::cout << "Initialising " << component_identifier.second->GetFullName() << std::endl;
		}
		Component* target_component = component_identifier.second;
		target_component->Initialise();
	}
	if (mg_verbose_output_flag) {
		std::cout << std::endl;
	}
	// Lastly we Solve() by iterating SubTick() until device state has stabilised.
	Solve();
	if (mg_verbose_output_flag) {
		std::cout << GenerateHeader("Starting state settled.") << std::endl << std::endl;
	}
}

void Device::Initialise() {
	for (const auto& out_state: m_out_pin_states) {
		m_out_pin_state_changed[out_state.first] = true;
	}
	if (m_parent_device_pointer->CheckIfQueuedToPropagateThisTick(m_name_hash) == false) {
		m_parent_device_pointer->AddToPropagateNextTick(m_name_hash);
	}
}

void Device::SubTick(int index) {
	if (mg_verbose_output_flag) {
		std::cout << "Iteration: " << std::to_string(index) << std::endl;
	}
	std::vector<std::size_t> propagate_this_tick;
	for (const auto& entry: m_propagate_next_tick) {		// entry.first = component name hash, entry.second = bool
		propagate_this_tick.push_back(entry.first);
	}
	m_propagate_next_tick.clear();
	for (const auto& entry: propagate_this_tick) {
		m_yet_to_propagate_this_tick[entry] = true;
	}
	for (const auto& entry: propagate_this_tick) {
		m_yet_to_propagate_this_tick[entry] = false;
		Component* current_component = m_components[entry];
		current_component->Propagate();
	}
	if (mg_verbose_output_flag) {
		std::cout << std::endl;
	}
}

void Device::Solve() {
	int sub_tick_count = 0;
	int sub_tick_limit = m_max_propagations;
	while (m_propagate_next_tick.size() > 0) {
		if (mg_verbose_output_flag) {
			std::cout << BOLD(FMAG("Level " << BOLD("" << m_nesting_level << "") << " Device " << BOLD("" << m_full_name << "") << " starting to Solve()...")) << std::endl << std::endl;
		}
		while (sub_tick_count <= sub_tick_limit) {
			SubTick(sub_tick_count);
			if (m_propagate_next_tick.size() > 0) {
				sub_tick_count += 1;
			} else {
				break;
			}
		}
		if (sub_tick_count > sub_tick_limit) {
			std::cout << "Could not stabilise " << m_full_name << " state within " << m_max_propagations << " propagation steps." << std::endl;
			break;
		} else {
			if (mg_verbose_output_flag) {
				std::cout << "...Solve()d." << std::endl << std::endl;
			}
		}
		for (const auto& component_identifier: m_components) {
			if (component_identifier.second->GetDeviceFlag()) {
				if ((component_identifier.second->m_monitor_on) && !(mg_verbose_output_flag)) {
					std::cout << std::endl;
				}
				// If this child Component is a child Device, recast the Component pointer to a Device pointer
				// and call the child Devices' Solve() method.
				Device* device_pointer = static_cast<Device*>(component_identifier.second);
				device_pointer->Solve();
			}
		}
	}
	if (this == m_top_level_sim_pointer) {
		// We check if we need to trigger probes here, rather than in the sim's Run() method after the sim's Solve()
		// returns, so that if we manually toggle the clock, the probes get triggered.
		m_top_level_sim_pointer->CheckProbeTriggers();
	}
}

void Device::Propagate() {
	for (const auto& origin_pin_name_hash: m_sorted_in_pin_name_hashes) {
		bool* state_changed = &m_in_pin_state_changed[origin_pin_name_hash];
		if (*state_changed) {
			bool* state = &m_in_pin_states[origin_pin_name_hash];
			if (mg_verbose_output_flag) {
				std::string pin_name = GetInPinName(origin_pin_name_hash);
				std::cout << BOLD(FGRN("->")) << " Device " << BOLD("" << m_full_name << "") << " propagating input " << pin_name << " = " << BoolToChar(*state) << std::endl;
			}
			*state_changed = false;
			std::unordered_map<std::size_t, connection_descriptor> connections_to_set = m_ports[origin_pin_name_hash];
			for (const auto& connection: connections_to_set) {
				Component* target_component = connection.second.target_component;
				std::size_t target_pin_name_hash = connection.second.target_pin_name_hash;
				target_component->Set(target_pin_name_hash, *state);
			}
		}
	}
	for (const auto& origin_pin_name_hash: m_sorted_out_pin_name_hashes) {
		bool* state_changed = &m_out_pin_state_changed[origin_pin_name_hash];
		if (*state_changed) {
			bool* state = &m_out_pin_states[origin_pin_name_hash];
			if (mg_verbose_output_flag) {
				std::string pin_name = GetOutPinName(origin_pin_name_hash);
				std::cout << BOLD(FRED("->")) << "Device " << BOLD("" << m_full_name << "") << " propagating output " << pin_name << " = " << BoolToChar(*state) << std::endl;
			}
			*state_changed = false;
			std::unordered_map<std::size_t, connection_descriptor> connections_to_set = m_ports[origin_pin_name_hash];
			for (const auto& connection: connections_to_set) {
				Component* target_component = connection.second.target_component;
				std::size_t target_pin_name_hash = connection.second.target_pin_name_hash;
				target_component->Set(target_pin_name_hash, *state);
			}
		}
	}
}

void Device::Connect(std::string const& origin_pin_name, std::string const& target_component_name, std::string const& target_pin_name) {
	std::size_t origin_pin_name_hash = std::hash<std::string>{}(origin_pin_name);
	std::size_t target_component_name_hash = std::hash<std::string>{}(target_component_name);
	std::size_t target_pin_name_hash = std::hash<std::string>{}(target_pin_name);
	std::size_t connection_identifier_hash = std::hash<std::string>{}(target_component_name + ":" + target_pin_name);
	bool connection_exists = IsHashInMapKeys(connection_identifier_hash, m_ports[origin_pin_name_hash]);
	if (connection_exists == false) {
		Component* target_component;
		if (IsHashInMapKeys(origin_pin_name_hash, m_in_pin_states)) {
			// If the device state is one of it's inputs, it can only be connected to an input terminal
			// of an internal child device.
			target_component = m_components[target_component_name_hash];
			connection_descriptor conn_descriptor;
			conn_descriptor.target_component = target_component;
			conn_descriptor.target_pin_name_hash = target_pin_name_hash;
			m_ports[origin_pin_name_hash][connection_identifier_hash] = conn_descriptor;
		} else {
			if (target_component_name == "parent") {
				// If the target is the parent device, then we are connecting to an output belonging to the
				// parent device.
				target_component = m_parent_device_pointer;
			} else {
				// If the target is not the parent device then it is another colleague device within the
				// same parent device.
				target_component = m_parent_device_pointer->GetChildComponentPointer(target_component_name);
			}
			connection_descriptor conn_descriptor;
			conn_descriptor.target_component = target_component;
			conn_descriptor.target_pin_name_hash = target_pin_name_hash;
			m_ports[origin_pin_name_hash][connection_identifier_hash] = conn_descriptor;
		}
	} else {
		std::cout << "Duplicate connection omitted." << std::endl;
	}
}

void Device::Set(std::size_t pin_name_hash, bool state_to_set) {
	int terminal_type = m_pin_directions[pin_name_hash];
	if (terminal_type == 1) {
		// Device input terminal is being set.
		bool* current_state = &m_in_pin_states[pin_name_hash];
		// If this would change the input terminal state, check if this triggers a magic event, then propagate this change
		// to all connected child component inputs.
		if (state_to_set != *current_state) {
			if (mg_verbose_output_flag) {
				std::string terminal_name = GetInPinName(pin_name_hash);
				std::cout << BOLD(FGRN("  ->")) << " Device " << BOLD("" << m_full_name << "") << " input terminal " << BOLD("" << terminal_name << "") << " set from " << BoolToChar(*current_state) << " to " << BoolToChar(state_to_set) << std::endl;
			}
			if (m_magic_device_flag == true) {
				m_magic_engine_pointer->CheckMagicEventTrap(pin_name_hash, state_to_set);
			}
			*current_state = state_to_set;
			//~m_in_pin_states[pin_name_hash] = state_to_set;
			m_in_pin_state_changed[pin_name_hash] = true;
			// Add device to the parent Devices propagate_next list, UNLESS this device
			// is already queued-up to propagate this SubTick.
			if (m_parent_device_pointer->CheckIfQueuedToPropagateThisTick(m_name_hash) == false) {
				m_parent_device_pointer->AddToPropagateNextTick(m_name_hash);
			}
			if ((m_monitor_on) || (mg_verbose_output_flag)) {								// Print input pin changes.
				std::string terminal_name = GetInPinName(pin_name_hash);
				std::cout << BOLD(FRED("  MONITOR: ")) << "Component " << BOLD("" << m_full_name << ":" << m_component_type << "") " input terminal " << BOLD("" << terminal_name << "") << " set to " << BoolToChar(state_to_set) << std::endl;
			}
		}
	} else if (terminal_type == 2) {
		// Device output terminal is being set.
		bool* current_state = &m_out_pin_states[pin_name_hash];
		if (state_to_set != *current_state) {
			if (mg_verbose_output_flag) {
				std::string terminal_name = GetOutPinName(pin_name_hash);
				std::cout << BOLD(FYEL("  ->")) << " Device " << BOLD("" << m_full_name << "") << " output terminal " << BOLD("" << terminal_name << "") << " set from " << BoolToChar(*current_state) << " to " << BoolToChar(state_to_set) << std::endl;
			}
			*current_state = state_to_set;
			//~m_out_pin_states[pin_name_hash] = state_to_set;
			m_out_pin_state_changed[pin_name_hash] = true;
			// Add device to the parent Devices propagate_next list, UNLESS this device
			// is already queued-up to propagate this SubTick.
			if (m_parent_device_pointer->CheckIfQueuedToPropagateThisTick(m_name_hash) == false) {
				m_parent_device_pointer->AddToPropagateNextTick(m_name_hash);
			}
			if ((m_monitor_on) || (mg_verbose_output_flag)) {								// Print output pin changes.
				std::string terminal_name = GetOutPinName(pin_name_hash);
				std::cout << BOLD(FRED("  MONITOR: ")) << "Component " << BOLD("" << m_full_name << ":" << m_component_type << "") " output terminal " << BOLD("" << terminal_name << "") << " set to " << BoolToChar(state_to_set) << std::endl;
			}
		}
	} else {
		if ((pin_name_hash == std::hash<std::string>{}("__ALL_STOP__")) && (m_top_level_sim_pointer->IsSimulationRunning())) {
			if (state_to_set) {
				std::cout << BOLD(" ---!--- Device " << m_full_name << " ALL_STOP was asserted ---!---") << std::endl;
				m_top_level_sim_pointer->StopSimulation();
			}
		}
	}
}

Component* Device::GetChildComponentPointer(std::string const& target_child_component_name) {
	std::size_t target_child_component_name_hash = std::hash<std::string>{}(target_child_component_name);
	return m_components[target_child_component_name_hash];
}

int Device::GetNestingLevel() {
	return m_nesting_level;
}

void Device::AddToPropagateNextTick(std::size_t propagation_identifier) {
	m_propagate_next_tick[propagation_identifier] = true;
}

bool Device::CheckIfQueuedToPropagateThisTick(std::size_t propagation_identifier) {
	if (m_yet_to_propagate_this_tick[propagation_identifier] == true) {
		return true;
	} else {
		return false;
	}
}

void Device::MakeProbable() {
	if (this != m_top_level_sim_pointer) {
		m_top_level_sim_pointer->AddToProbableDevices(m_full_name, this);
	}
}

void Device::PrintPinStates(int max_levels) {
	if (max_levels > 0) {
		max_levels = max_levels - 1;
		std::cout << m_full_name << ": [";
		for (const auto& pin_name: m_sorted_in_pin_names) {
			if (!IsStringInVector(pin_name, m_hidden_in_pins)) {
				std::size_t pin_name_hash = std::hash<std::string>{}(pin_name);
				std::cout << " " << BoolToChar(m_in_pin_states[pin_name_hash]) << " ";
			}
		}
		std::cout << "] [";
		for (const auto& pin_name: m_sorted_out_pin_names) {
			std::size_t pin_name_hash = std::hash<std::string>{}(pin_name);
			std::cout << " " << BoolToChar(m_out_pin_states[pin_name_hash]) << " ";
		}
		std::cout << "]" << std::endl;
		if (max_levels > 0) {
			PrintInternalPinStates(max_levels);
		}
	}
}

void Device::PrintInternalPinStates(int max_levels) {
	for (const auto& component: m_components) {
		int this_level = max_levels;
		//std::string device_identifier = entry.first;
		Component* target_component = component.second;
		target_component->PrintPinStates(this_level);
	}
}
