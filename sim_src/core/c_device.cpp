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

#include <string>					// std::string
#include <iostream>					// std::cout, std::endl
#include <vector>					// std::vector
#include <algorithm>				// std::sort
#include <thread>					// std::thread
#include <mutex>					// std::mutex, std::unique_lock
#include <functional>				// std::bind

#include "c_core.h"					// Core simulator functionality
#include "utils.h"
#include "strnatcmp.h"
#include "colors.h"

bool backwards_comp (int i, int j) {
	return (j < i);
}

Device::Device(Device* parent_device_pointer, std::string const& device_name, std::string const& device_type, std::vector<std::string> in_pin_names,
	std::vector<std::string> out_pin_names, bool monitor_on, std::vector<state_descriptor> in_pin_default_states, int max_propagations
	) {
	m_device_flag = true;
	m_name = device_name;
	m_parent_device_pointer = parent_device_pointer;
	if (m_parent_device_pointer == this) {
		// The instantiating component is only the same as it's parent component when it is the top-level Simulation.
		m_top_level_sim_pointer = static_cast<Simulation*>(m_parent_device_pointer);
		m_CUID = 0;						// Top-level simulation always CUID 0.
		m_nesting_level = 0;			// "" nesting_level 0
		m_full_name = m_name;
	} else {
		m_top_level_sim_pointer = m_parent_device_pointer->GetTopLevelSimPointer();
		m_CUID = m_top_level_sim_pointer->GetNewCUID();
		m_local_component_index = m_parent_device_pointer->GetNewLocalComponentIndex();
		m_nesting_level = m_parent_device_pointer->GetNestingLevel() + 1;
		m_full_name = m_parent_device_pointer->GetFullName() + ":" + m_name;
		m_parent_device_pointer->CreateChildFlags();
	}
	m_component_type = device_type;
	m_monitor_on = monitor_on;
	m_solve_children_in_own_threads = (m_top_level_sim_pointer->m_use_threaded_solver && (m_nesting_level == m_top_level_sim_pointer->m_threaded_solve_nesting_level));
	if (max_propagations == 0) {
		// If default max_propagations get the value from the top-level simulation.
		m_max_propagations = m_top_level_sim_pointer->GetTopLevelMaxPropagations();
	} else {
		m_max_propagations = max_propagations;
	}
	std::sort(in_pin_names.begin(), in_pin_names.end(), compareNat);
	// Utility 'hidden' in and out pins have to be created by the base Device constructor.
	// Further calls by Devices inheriting the base class won't include these.
	in_pin_names.insert(in_pin_names.end(), m_hidden_in_pins.begin(), m_hidden_in_pins.end());
	CreateInPins(in_pin_names, in_pin_default_states);
	m_in_pin_default_states = in_pin_default_states;
	out_pin_names.insert(out_pin_names.end(), m_hidden_out_pins.begin(), m_hidden_out_pins.end());
	CreateOutPins(out_pin_names);
}

Device::~Device() {
	PurgeComponent();
	if (mg_verbose_destructor_flag) {
		std::cout << "Device dtor for " << m_full_name << " @ " << this << std::endl;
	}
}

void Device::CreateChildFlags() {
	// Create un-set propagation flags for a new child Device.
	m_propagate_next_tick_flags.push_back(false);
	m_propagate_this_tick_flags.push_back(false);
}

void Device::CreateInPins(std::vector<std::string> const& pin_names, std::vector<state_descriptor> pin_default_states) {
	// Determine number of existing in and out pins.
	int new_pin_port_index = m_pins.size();
	// Create new inputs.
	for (const auto& pin_name: pin_names) {
		pin new_in_pin = {pin_name, 1, false, false, new_pin_port_index, {false, false}};
		SetPin(new_in_pin, pin_default_states);
		m_pins.push_back(new_in_pin);
		m_ports.push_back({});
		new_pin_port_index ++;
	}
}

void Device::CreateOutPins(std::vector<std::string> const& pin_names) {
	// Determine number of existing in and out pins.
	int new_pin_port_index = m_pins.size();
	// Create new outputs.
	for (const auto& pin_name : pin_names) {
		pin new_out_pin = {pin_name, 2, false, false, new_pin_port_index, {false, false}};
		SetPin(new_out_pin, {});
		m_pins.push_back(new_out_pin);
		m_ports.push_back({});
		new_pin_port_index ++;
	}
}

void Device::CreateBus(int pin_count, std::string const& pin_name_prefix, int pin_direction, std::vector<state_descriptor> in_pin_default_states) {
	std::vector<std::string> pins_to_create = {};
	for (int pin_index = 0; pin_index < pin_count; pin_index ++) {
		std::string pin_name = pin_name_prefix + std::to_string(pin_index);
		pins_to_create.push_back(pin_name);
	}
	if (pin_direction == 1) {
		CreateInPins(pins_to_create, in_pin_default_states);
	} else if (pin_direction == 2) {
		CreateOutPins(pins_to_create);
	} else {
		std::string build_error = "Device " + m_full_name + " tried to create a bus with pin_direction = " + std::to_string(pin_direction) + " but this is not possible.";
		m_top_level_sim_pointer->LogError(build_error);
	}
}

void Device::SetPin(pin& target_pin, std::vector<state_descriptor> pin_default_states) {
	// SetPin() only sets pin logical state & state_changed flag for input and output (direction = 1 or 2) pins.
	std::string pin_name = target_pin.name;
	if (IsStringInVector(pin_name, m_hidden_in_pins)) {
		// If hidden in pin, set direction and state accordingly...
		target_pin.direction = 3;
		target_pin.state_changed = true;
		if (pin_name == "true") {
			target_pin.state = true;
		} else if (pin_name == "false") {
			target_pin.state = false;
		}
	} else if (IsStringInVector(pin_name, m_hidden_out_pins)) {
		// ...likewise for hidden out pins.
		target_pin.direction = 4;
		target_pin.state_changed = false;
		target_pin.state = false;
	} else {
		// If this is a user-defined input, handle as normal.
		std::vector<bool> result = IsStringInStateDescriptorVector(pin_name, pin_default_states);
		if (result[0]) {
			// If this input is in the defaults list set accordingly...
			target_pin.state = result[1];
		} else {
			// ...otherwise set input state to false.
			target_pin.state = false;
		}
		target_pin.state_changed = true;
	}
}

void Device::Reset() {
	// Recursively explore the whole Component tree resetting all pins to their starting states and then call Stabilise().
	// First, ensure that we run the Reset() from the top-level of the Simulation, irrespective of the initial caller.
	if ((!m_top_level_sim_pointer->GetSearchingFlag()) && (this != m_top_level_sim_pointer)) {
		m_top_level_sim_pointer->SetSearchingFlag(true);
		m_top_level_sim_pointer->Reset();
	} else {
		if ((!m_top_level_sim_pointer->GetSearchingFlag()) && (this == m_top_level_sim_pointer)) {
			m_top_level_sim_pointer->SetSearchingFlag(true);
		}
		// We want to Reset() everything in reverse-depth order, starting from the very end of each branch,
		// so first we Search deeper if we can.
		for (const auto& this_device_local_id : m_devices) {
			m_components[this_device_local_id].component_pointer->Reset();
		}
		// Once we bottom-out, we reset this Device's pins, and then reset all the sibling Gates.
		// Set all Device in and out pins to their default starting states.
		for (auto& this_pin : m_pins) {
			SetPin(this_pin, m_in_pin_default_states);
		}
		// Loop over child Gates and reset their in and out pins.
		for (const auto& this_component_descriptor : m_components) {
			if (!this_component_descriptor.component_pointer->GetDeviceFlag()) {
				this_component_descriptor.component_pointer->Reset();
			}
		}
		// Finally, call Stabilise() for this Device. 
		Stabilise();
	}
	// If we are 'coming back up for air' arriving at the top-level Simulation, reset the searching flag.
	if ((m_top_level_sim_pointer->GetSearchingFlag()) && (this == m_top_level_sim_pointer)) {
		m_top_level_sim_pointer->SetSearchingFlag(false);
	}
}

void Device::Stabilise() {
	// Ensures that internal device state settles correctly.
	if (mg_verbose_flag) {
		std::string message = GenerateHeader("Stabilising new level " + std::to_string(m_nesting_level) + " Device " + m_full_name);
		m_top_level_sim_pointer->LogMessage(message + "\n");
	}
	// First we call Initialise() for all Components to set their out pins state_changed flags to true and
	// add their local ids to this Device's propagate next vector.
	for (const auto& this_component_descriptor : m_components) {
			if (mg_verbose_flag) {
				std::string message = "Initialising " + this_component_descriptor.component_full_name + "...";
				m_top_level_sim_pointer->LogMessage(message);
			}
			this_component_descriptor.component_pointer->Initialise();
	}
	// Then we call Solve() for this Device.
	Solve(false, 0);
	if (mg_verbose_flag) {
		std::string message = "\n" + GenerateHeader("Starting state settled.") + "\n";
		m_top_level_sim_pointer->LogMessage(message);
	}
	if (this == m_top_level_sim_pointer) {
		ReportUnConnectedPins();
		std::string message = "\n" + GenerateHeader("Simulation build completed.");
		m_top_level_sim_pointer->LogMessage(message);
		m_top_level_sim_pointer->PrintAndClearMessages();
		m_top_level_sim_pointer->PrintErrorMessages();
	}
}

void Device::Initialise() {
	// We need to make sure that all Device out pins get propagated during the parent Device's Stabilise()
	// call at the end of it's Build() call. This ensures that Sibling Gates that are set by the out pins of
	// this Device - that then set in pins of other sibling Devices - have their in pins set to the correct
	// states. Ordinarily we only want to propagate the Device out pin states if they have changed, here they
	// all need to be propagated so that they 'overwrite' the initial random build-time Gate inputs.
	for (auto& this_pin : m_pins) {
		if (this_pin.direction == 2) {
			this_pin.state_changed = true;
		}
	}
	m_parent_device_pointer->AppendChildPropagationIdentifier(m_local_component_index);
}

void Device::AddComponent(Component* new_component_pointer) {
	std::string new_component_name = new_component_pointer->GetName();
	std::string new_component_full_name = new_component_pointer->GetFullName();
	component_descriptor new_component_descriptor;
	new_component_descriptor.component_name = new_component_name;
	new_component_descriptor.component_full_name = new_component_full_name;
	new_component_descriptor.component_pointer = new_component_pointer;
	m_components.push_back(new_component_descriptor);
	// Keep an additional vector of the local component indices of Device Components so that we don't
	// have to iterate over all Components to call Solve() on Devices, during the Solve() call.
	if (new_component_pointer->GetDeviceFlag()) {
		m_devices.push_back(new_component_pointer->GetLocalComponentIndex());
	}
}

void Device::AddGate(std::string const& component_name, std::string const& component_type, std::vector<std::string> const& in_pin_names, bool monitor_on) {
	AddComponent(new Gate(this, component_name, component_type, in_pin_names, monitor_on));
}

void Device::AddGate(std::string const& component_name, std::string const& component_type, bool monitor_on) {
	AddComponent(new Gate(this, component_name, component_type, {}, monitor_on));
}

void Device::AddMagicEventTrap(std::string const& target_pin_name, std::vector<bool> const& state_change, std::vector<human_writable_magic_event_co_condition> const& hw_co_conditions, int incantation) {
	if (m_magic_device_flag == true) {
		// Convert the human-writable  kind of magic event co-condition (strings for terminal identifiers) to the kind pin-indexed kind.
		std::vector<magic_event_co_condition> co_conditions = {};
		for (size_t i = 0; i  < hw_co_conditions.size(); i ++) {
			magic_event_co_condition this_co_condition;
			this_co_condition.pin_port_index = GetPinPortIndex(hw_co_conditions[i].pin_name);
			this_co_condition.pin_state = hw_co_conditions[i].pin_state;
			co_conditions.push_back(this_co_condition);
		}
		magic_event new_magic_event;
		new_magic_event.target_pin_port_index = GetPinPortIndex(target_pin_name);
		new_magic_event.state_change = state_change;
		new_magic_event.co_conditions = co_conditions;
		new_magic_event.incantation = incantation;
		// Ensure that m_magic_pin_flag is set for this pin.  ---------------------------------------------------------
		// If this is the first time this has been done we need to initialise at-least enough entries for all in pins.
		if (m_magic_pin_flag.size() == 0) {
			int max_in_pin_port_index = 0;
			for (const auto& this_pin : m_pins) {
				if (this_pin.direction == 1) {
					if (this_pin.port_index > max_in_pin_port_index) {
						max_in_pin_port_index = this_pin.port_index;
					}
				}
			}
			for (int pin_index = 0; pin_index < max_in_pin_port_index + 1; pin_index ++) {
				m_magic_pin_flag.push_back(false);
			}
		}
		m_magic_pin_flag[new_magic_event.target_pin_port_index] = true;
		// -----------------------------------------------------------------------------------------------------------
		m_magic_engine_pointer->AddMagicEventTrap(new_magic_event);
	} else {
		std::cout << "Device " + m_full_name + " is not magic! Cannot add magic event trap." << std::endl;
	}
}

void Device::ChildConnect(std::string const& target_child_component_name, std::vector<std::string> const& connection_parameters) {
	Component* target_component_pointer = GetChildComponentPointer(target_child_component_name);
	if (target_component_pointer == 0) {
		// Log build error here.		-- Child Component does not exist.
		std::string build_error = "Device " + m_full_name + " tried to connect from child Component " + target_child_component_name + " but it does not exist.";
		m_top_level_sim_pointer->LogError(build_error);
	} else {
		target_component_pointer->Connect(connection_parameters);
	}
}

void Device::ChildSet(std::string const& target_child_component_name, std::string const& target_pin_name, bool logical_state) {
	Component* target_component_pointer = GetChildComponentPointer(target_child_component_name);
	if (target_component_pointer == 0) {
		// Log build error here.		-- Child Component does not exist.
		std::string build_error = "Device " + m_full_name + " tried to Set() an input of child Component " + target_child_component_name + " but it does not exist.";
		m_top_level_sim_pointer->LogError(build_error);
	} else {
		int target_pin_port_index = target_component_pointer->GetPinPortIndex(target_pin_name);
		bool target_component_monitor_on_flag = target_component_pointer->GetMonitorOnFlag();
		if (mg_verbose_flag || target_component_monitor_on_flag) {
			std::cout << std::endl << BOLD(FYEL("CHILDSET: ")) << "Component " << BOLD("" << target_component_pointer->GetFullName() << ":" << target_component_pointer->GetComponentType() << "") << " terminal " << BOLD("" << target_pin_name << "") << " set to " << BoolToChar(logical_state) << std::endl;
		}
		if (mg_verbose_flag) {
			m_top_level_sim_pointer->LogMessage("~S0");
		}
		target_component_pointer->Set(target_pin_port_index, logical_state);
		if (mg_verbose_flag) {
			m_top_level_sim_pointer->LogMessage("~E0");
		}
		// If this is a 1st-level device, if the simulation is not running the user would need to call Solve() after every
		// 'manual' pin change to make sure that 1st-level device state is propagated. Instead, we check for them here if
		// the simulation is not running and call Solve() at the end of the Set() call.
		if (!(m_top_level_sim_pointer->IsSimulationRunning())) {
			m_top_level_sim_pointer->Solve(false, 0);
			m_top_level_sim_pointer->PrintAndClearMessages();
		}
	}
}

void Device::ChildPrintPinStates(std::string const& target_child_component_name, int max_levels) {
	Component* target_component_pointer = GetChildComponentPointer(target_child_component_name);
	if (target_component_pointer == 0) {
		// Log build error here.		-- Child Component does not exist.
		std::string build_error = "Device " + m_full_name + " tried to recursively print pin states from child Component " + target_child_component_name + " but it does not exist.";
		m_top_level_sim_pointer->LogError(build_error);
	} else {
		target_component_pointer->PrintPinStates(max_levels);
	}
}

void Device::ChildPrintInPinStates(std::string const& target_child_component_name) {
	Component* target_component_pointer = GetChildComponentPointer(target_child_component_name);
	if (target_component_pointer == 0) {
		// Log build error here.		-- Child Component does not exist.
		std::string build_error = "Device " + m_full_name + " tried to print in pin states for child Component " + target_child_component_name + " but it does not exist.";
		m_top_level_sim_pointer->LogError(build_error);
	} else {
		target_component_pointer->PrintInPinStates();
	}
}

void Device::ChildPrintOutPinStates(std::string const& target_child_component_name) {
	Component* target_component_pointer = GetChildComponentPointer(target_child_component_name);
	if (target_component_pointer == 0) {
		// Log build error here.		-- Child Component does not exist.
		std::string build_error = "Device " + m_full_name + " tried to print out pin states for child Component " + target_child_component_name + " but it does not exist.";
		m_top_level_sim_pointer->LogError(build_error);
	} else {
		target_component_pointer->PrintOutPinStates();
	}
}

void Device::ChildMarkOutputNotConnected(std::string const& target_child_component_name, std::string const& target_out_pin_name) {
	Component* target_component_pointer = GetChildComponentPointer(target_child_component_name);
	if (target_component_pointer == 0) {
		// Log build error here.		-- Child Component does not exist.
		std::string build_error = "Device " + m_full_name + " tried to mark child Component " + target_child_component_name + " output pin " + target_out_pin_name + " not connected but the component does not exist.";
		m_top_level_sim_pointer->LogError(build_error);
	} else {
		bool found = false;
		for (const auto& this_pin_name : target_component_pointer->GetSortedOutPinNames()) {
			if (this_pin_name == target_out_pin_name) {
				int target_pin_port_index = target_component_pointer->GetPinPortIndex(this_pin_name);
				target_component_pointer->SetPinDrivenFlag(target_pin_port_index, true, true);
				found = true;
				break;
			}
		}
		if (!found) {
			// Log build error here.		-- Output pin does not exist.
			std::string build_error = "Device " + m_full_name + " tried to mark child Component " + target_child_component_name + " output pin " + target_out_pin_name + " not connected but the pin does not exist.";
			m_top_level_sim_pointer->LogError(build_error);
		}
	}
}

void Device::Connect(std::string const& origin_pin_name, std::string const& target_component_name, std::string const& target_pin_name) {
	std::vector<std::string> connection_parameters = {origin_pin_name, target_component_name, target_pin_name};
	Connect(connection_parameters);
}

void Device::Connect(std::vector<std::string> connection_parameters) {
	// The below seems to work fine, but it is very untidy and hard to follow and *needs refactoring*.
	if ((connection_parameters.size() == 2) || (connection_parameters.size() == 3)) {
		std::string origin_pin_name = connection_parameters[0];
		bool origin_pin_exists = CheckIfPinExists(origin_pin_name);
		if (origin_pin_exists) {
			bool target_component_exists = true;
			std::string target_component_nature_string = "";
			bool target_pin_exists = true;
			int target_pin_direction = 0;
			bool target_pin_direction_compatible = false;
			bool no_existing_connection = true;
			std::vector<bool> target_pin_already_driven;
			
			std::string target_component_name = connection_parameters[1];
			std::string target_pin_name;
			if (connection_parameters.size() == 2) {
				target_pin_name = "input";
			} else {
				target_pin_name = connection_parameters[2];
			}
			
			int origin_pin_direction = GetPinDirection(origin_pin_name);
			std::string origin_direction = "";
			if (origin_pin_direction == 1) {
				origin_direction = "input";
			} else if (origin_pin_direction == 2) {
				origin_direction = "output";
			} else if (origin_pin_direction == 3) {
				origin_direction = "hidden input";
			} else if (origin_pin_direction == 4) {
				origin_direction = "hidden output";
			}
			
			int origin_pin_port_index = GetPinPortIndex(origin_pin_name);
			std::vector<int> required_target_directions;
			Component* target_component_pointer = 0;
			if ((origin_pin_direction == 1) || (origin_pin_direction == 3)) {
				// If the device state is one of it's inputs, it can only be connected to an input terminal
				// of an internal child device.
				required_target_directions = {1};
				target_component_nature_string = "child";
				target_component_pointer = GetChildComponentPointer(target_component_name);
				if (target_component_pointer == 0) {
					target_component_exists = false;
				}
			} else if (origin_pin_direction == 2) {
				if (target_component_name == "parent") {
					// If the target is the parent device, then we are connecting to an output belonging to the
					// parent device.
					required_target_directions = {2, 4};
					target_component_nature_string = "parent";
					target_component_pointer = m_parent_device_pointer;
				} else {
					// If the target is not the parent device then it is another colleague device within the
					// same parent device.
					required_target_directions = {1};
					target_component_nature_string = "sibling";
					target_component_pointer = m_parent_device_pointer->GetChildComponentPointer(target_component_name);
					if (target_component_pointer == 0) {
						target_component_exists = false;
					}
				}
			} else {
				// Pin is a hidden output and cannot be connected onwards...
			}
			
			if (target_component_exists) {
				target_pin_exists = target_component_pointer->CheckIfPinExists(target_pin_name);
				if (target_pin_exists) {
					connection_descriptor new_connection_descriptor;
					new_connection_descriptor.target_component_pointer = target_component_pointer;
					new_connection_descriptor.target_pin_port_index = target_component_pointer->GetPinPortIndex(target_pin_name);
					target_pin_direction = target_component_pointer->GetPinDirection(new_connection_descriptor.target_pin_port_index);
					for (const auto& this_required_direction : required_target_directions) {
						if (this_required_direction == target_pin_direction) {
							target_pin_direction_compatible = true;
							break;
						}
					}
					if (target_pin_direction_compatible) {
						for (const auto& this_connection_descriptor : m_ports[origin_pin_port_index]) {
							if ((this_connection_descriptor.target_pin_port_index == new_connection_descriptor.target_pin_port_index) && (this_connection_descriptor.target_component_pointer == new_connection_descriptor.target_component_pointer)) {
								no_existing_connection = false;
								break;
							}
						}
						if (no_existing_connection) {
							target_pin_already_driven = target_component_pointer->CheckIfPinDriven(new_connection_descriptor.target_pin_port_index);
							if (!target_pin_already_driven[0]) {
								m_ports[origin_pin_port_index].push_back(new_connection_descriptor);
								target_component_pointer->SetPinDrivenFlag(new_connection_descriptor.target_pin_port_index, false, true);
								m_pins[origin_pin_port_index].drive[1] = true;
							}
						}
					}
				}
			}
			
			std::string target_direction = "";
			if (target_pin_direction == 1) {
				target_direction = "input";
			} else if (target_pin_direction == 2) {
				target_direction = "output";
			} else if (target_pin_direction == 3) {
				target_direction = "hidden input";
			} else if (target_pin_direction == 4) {
				target_direction = "hidden output";
			}
			
			if (!target_component_exists) {
				// Log build error here.		-- Component does not exist.
				std::string build_error = "Device " + m_full_name + " tried to connect " + origin_direction + " " + origin_pin_name + " to " + target_component_nature_string + " component " + target_component_name + " but it does not exist.";
				m_top_level_sim_pointer->LogError(build_error);
			} else {
				if (!target_pin_exists) {
					// Log build error here.		-- Target pin does not exist.
					std::string build_error = "Device " + m_full_name + " tried to connect " + origin_direction + " " + origin_pin_name + " to " + target_component_nature_string + " component " + target_component_name + " pin " + target_pin_name + " but it does not exist.";
					m_top_level_sim_pointer->LogError(build_error);
				} else {
					if (!target_pin_direction_compatible) {
						// Log build error here.		-- Origin and target pin directions are not compatible.
						std::string build_error = "Device " + m_full_name + " tried to connect " + origin_direction + " " + origin_pin_name + " to " + target_component_nature_string + " component " + target_component_name + " " + target_direction + " pin " + target_pin_name + " but they are not compatible";
						m_top_level_sim_pointer->LogError(build_error);
					} else {
						if (!no_existing_connection) {
							// Log build error here.		-- This connection already exists.
							std::string build_error = "Device " + m_full_name + " tried to connect " + origin_direction + " " + origin_pin_name + " to " + target_component_nature_string + " component " + target_component_name + " pin " + target_pin_name + " but is already connected to it.";
							m_top_level_sim_pointer->LogError(build_error);
						} else {
							if (target_pin_already_driven[0]) {
								// Log build error here.		-- This target pin is already driven by another pin.
								std::string build_error = "Device " + m_full_name + " tried to connect " + origin_direction + " " + origin_pin_name + " to " + target_component_nature_string + " component " + target_component_name + " pin " + target_pin_name + " but it is already driven by another pin.";
								m_top_level_sim_pointer->LogError(build_error);
							}
						}
					}
				}
			}
		} else {
			// Log build error here.		-- Origin pin does not exist.
			std::string build_error = "Device " + m_full_name + " tried to connect from input " + origin_pin_name + " but it does not exist.";
			m_top_level_sim_pointer->LogError(build_error);
		}
	} else {
		// Log build error here.		-- Wrong number of connection parameters.
		std::string build_error = "Device " + m_full_name + " tried to form a connection but the wrong number of connection parameters were provided.";
		m_top_level_sim_pointer->LogError(build_error);
	}
}

void Device::Solve(const bool threaded_solve, const int branch_id) {
	int original_branch_id = m_message_branch_id;
	if (threaded_solve) {
		m_message_branch_id = m_CUID;
	} else {
		m_message_branch_id = branch_id;	
	}
	if (mg_verbose_flag) {
		// Indicate the start of this solution level to the message handler.
		m_top_level_sim_pointer->LogMessage("~S" + std::to_string(m_message_branch_id));
		std::string message = std::string("\n") + std::to_string(m_message_branch_id) + ": " + KBLD + KMAG + "Level " + RST + KBLD + std::to_string(m_nesting_level) + RST + " Device " + KBLD + m_full_name + RST + " Propagating inputs...\n" + std::to_string(m_message_branch_id) + ":";
		m_top_level_sim_pointer->LogMessage("~" + std::to_string(m_message_branch_id) + ": " + message);
	}
	// Clear the Solve() pending flag.
	m_solve_this_tick_flag = false;
	int sub_tick_count = 0;
	// Propagate Device inputs first.
	PropagateInputs();
	while (true) {
		if (mg_verbose_flag) {
			std::string message = std::string("\n") + std::to_string(m_message_branch_id) + ": " + KBLD + KMAG + "Level " + RST + KBLD + std::to_string(m_nesting_level) + RST + " Device " + KBLD + m_full_name + RST + " starting to Solve()...";
			m_top_level_sim_pointer->LogMessage("~" + std::to_string(m_message_branch_id) + ": " + message); 
		}
		// Handle pending propagations for child Gates and child Device out pins.
		while (sub_tick_count <= m_max_propagations) {
			SubTick(sub_tick_count);
			// Terminate loop when there are no pending propagations.
			if (m_propagate_next_tick.size() > 0) {
				sub_tick_count ++;
			} else {
				sub_tick_count = 0;
				break;
			}
		}
		if (sub_tick_count > m_max_propagations) {
			// Log error here.		-- Not able to stabilise Device state.
			std::string error_message = "Could not stabilise " + m_full_name + " state within " + std::to_string(m_max_propagations) + " propagation steps.";
			m_top_level_sim_pointer->LogError("~" + std::to_string(m_message_branch_id) + ": " + error_message);
			if (mg_verbose_flag) {
				m_top_level_sim_pointer->LogMessage("~" + std::to_string(m_message_branch_id) + ": " + error_message);
				// Indicate the end (for now) of this solution level to the message handler.
				m_top_level_sim_pointer->LogMessage("~E" + std::to_string(m_message_branch_id));
			}
			break;
		} else {
			if (mg_verbose_flag) {
				std::string message = std::string("\n") + std::to_string(m_message_branch_id) + ": " + KBLD + KMAG + "Level " + RST + KBLD + std::to_string(m_nesting_level) + RST + " Device " + KBLD + m_full_name + RST + " Solve()d.";
				m_top_level_sim_pointer->LogMessage("~" + std::to_string(m_message_branch_id) + ": " + message);
				// Indicate the end (for now) of this solution level to the message handler.
				m_top_level_sim_pointer->LogMessage("~E" + std::to_string(m_message_branch_id));
			}
		}
		// ------------------------------------------------------------------------------------------------------
		if (!m_solve_children_in_own_threads) {
			// Regular Solve() for all pending child Devices.
			for (const auto& this_local_device_index : m_solve_this_tick) {
				Device* this_device_pointer = static_cast<Device*>(m_components[this_local_device_index].component_pointer);
				this_device_pointer->Solve(false, m_message_branch_id);
			}
		} else {	// Experimental multi-threading support -----------------------------------------------------------------------------
			// Threaded Solve() for all pending child Devices.
			for (const auto& this_local_device_index : m_solve_this_tick) {
				Device* this_device_pointer = static_cast<Device*>(m_components[this_local_device_index].component_pointer);
				m_top_level_sim_pointer->m_thread_pool_pointer->AddJob(std::bind(&Device::Solve, this_device_pointer, true, 0));
			}
			// Wait until all Device Solve() threads finish.
			m_top_level_sim_pointer->m_thread_pool_pointer->WaitForAllJobs();
		}
		// ------------------------------------------------------------------------------------------------------
		m_solve_this_tick.clear();
		// If there are no pending propagations within this Device following the child Device solve loop
		// above, we're all done here.
		if (m_propagate_next_tick.size() == 0) {
			break;
		} else {
			// If we have changes to propagate, indicate to the message handler that we're starting another pass at this level.
			if (mg_verbose_flag) {
				m_top_level_sim_pointer->LogMessage("~S" + std::to_string(m_message_branch_id));
			}
		}
	}
	// If we're solving the top-level Simulation state, we need to check if the Clock has triggered any Probes.
	if (this == m_top_level_sim_pointer) {
		m_top_level_sim_pointer->CheckProbeTriggers();
	} else {
		if (m_buffered_propagation) {
			m_buffered_propagation = false;
			m_parent_device_pointer->AppendChildPropagationIdentifier(m_local_component_index);
		}
	}
	m_message_branch_id = original_branch_id;
}

void Device::SubTick(const int index) {
	if (mg_verbose_flag) {
		std::string message = std::string("\n" + std::to_string(m_message_branch_id) + ": " + "Iteration: ") + std::to_string(index);
		m_top_level_sim_pointer->LogMessage("~" + std::to_string(m_message_branch_id) + ": " + message);
	}
	// ------------------------------------------
	for (const auto& this_entry : m_propagate_next_tick) {
		m_propagate_this_tick.emplace_back(this_entry);
		m_propagate_this_tick_flags[this_entry] = true;
		m_propagate_next_tick_flags[this_entry] = false;
	}
	m_propagate_next_tick.clear();
	for (const auto& this_entry : m_propagate_this_tick) {
		m_propagate_this_tick_flags[this_entry] = false;
		m_components[this_entry].component_pointer->Propagate();
	}
	m_propagate_this_tick.clear();
	// ------------------------------------------
}

void Device::AppendChildPropagationIdentifier(const int propagation_identifier) {
	if (!m_solve_children_in_own_threads) {
		m_propagate_next_tick.emplace_back(propagation_identifier);
	} else {
		std::unique_lock<std::mutex> lock(m_propagation_lock);
		m_propagate_next_tick.emplace_back(propagation_identifier);
	}
}

void Device::QueueToSolve(const int local_component_identifier) {
	m_solve_this_tick.emplace_back(local_component_identifier);
} 

void Device::QueueToPropagate(const int propagation_identifier) {
	if ((!m_propagate_this_tick_flags[propagation_identifier]) && (!m_propagate_next_tick_flags[propagation_identifier])) {
		m_propagate_next_tick.emplace_back(propagation_identifier);
		m_propagate_next_tick_flags[propagation_identifier] = true;
	}
}

void Device::PropagateInputs() {
	for (auto& this_pin : m_pins) {
		if ((this_pin.direction == 1) || (this_pin.direction == 3)) {
			if (this_pin.state_changed) {
				if (mg_verbose_flag) {
					std::string message = std::string(KBLD) + KBLU + "->" + RST + " Device " + KBLD + m_full_name + RST + " propagating input " + this_pin.name + " = " + BoolToChar(this_pin.state);
					m_top_level_sim_pointer->LogMessage("~" + std::to_string(m_message_branch_id) + ": " + message);
				}
				this_pin.state_changed = false;
				for (const auto& this_connection_descriptor : m_ports[this_pin.port_index]) {
					this_connection_descriptor.target_component_pointer->Set(this_connection_descriptor.target_pin_port_index, this_pin.state);
				}
			}
		}
	}
}

void Device::Propagate() {
	for (auto& this_pin : m_pins) {
		if (this_pin.direction == 2) {
			if (this_pin.state_changed) {
				if (mg_verbose_flag) {
					std::string message = std::string(KBLD) + KYEL + "->" + RST + " Device " + KBLD + m_full_name + RST + " propagating output " + this_pin.name + " = " + BoolToChar(this_pin.state);
					m_top_level_sim_pointer->LogMessage("~" + std::to_string(m_message_branch_id) + ": " + message);
				}
				this_pin.state_changed = false;
				for (const auto& this_connection_descriptor : m_ports[this_pin.port_index]) {
					this_connection_descriptor.target_component_pointer->Set(this_connection_descriptor.target_pin_port_index, this_pin.state);
				}
			}
		}
	}
}

void Device::Set(const int pin_port_index, const bool state_to_set) {
	pin* this_pin = &m_pins[pin_port_index];
	if (this_pin->direction == 1) {
		if (state_to_set != this_pin->state) {
			if ((m_monitor_on) || (mg_verbose_flag)) {
				std::string monitor_message = "";
				if (mg_verbose_flag) {
					std::string message = std::string(KBLD) + KGRN + "  ->" + RST + " Device " + KBLD + m_full_name + RST + " input terminal " + KBLD + this_pin->name + RST + " set from " + BoolToChar(this_pin->state) + " to " + BoolToChar(state_to_set);
					m_top_level_sim_pointer->LogMessage("~" + std::to_string(m_message_branch_id) + ": " + message);
					monitor_message = "~" + std::to_string(m_message_branch_id) + ": ";
				}
				monitor_message += std::string(KBLD) + KRED + "  MONITOR: " + RST + "Component " + KBLD + m_full_name + ":" + m_component_type + RST + " input terminal " + KBLD + this_pin->name + RST + " set to " + BoolToChar(state_to_set);
				//~m_top_level_sim_pointer->LogMessage("~" + std::to_string(m_message_branch_id) + ": " + message);
				m_top_level_sim_pointer->LogMessage(monitor_message);
			}
			if (m_magic_device_flag == true) {
				if (m_magic_pin_flag[pin_port_index]) {
					m_magic_engine_pointer->CheckMagicEventTrap(pin_port_index, state_to_set);
				}
			}
			this_pin->state = state_to_set;
			this_pin->state_changed = true;
			if (!m_solve_this_tick_flag) {
				m_solve_this_tick_flag = true;
				m_parent_device_pointer->QueueToSolve(m_local_component_index);
			}
		}
	} else if (this_pin->direction == 2) {
		if (state_to_set != this_pin->state) {
			if ((m_monitor_on) || (mg_verbose_flag)) {
				std::string monitor_message = "";
				if (mg_verbose_flag) {
					std::string message = std::string(KBLD) + KRED + "  ->" + RST + " Device " + KBLD + m_full_name + RST + " output terminal " + KBLD + this_pin->name + RST + " set from " + BoolToChar(this_pin->state) + " to " + BoolToChar(state_to_set);
					m_top_level_sim_pointer->LogMessage("~" + std::to_string(m_message_branch_id) + ": " + message);
					monitor_message = "~" + std::to_string(m_message_branch_id) + ": ";
				}
				monitor_message += std::string(KBLD) + KRED + "  MONITOR: " + RST + "Component " + KBLD + m_full_name + ":" + m_component_type + RST + " output terminal " + KBLD + this_pin->name + RST + " set to " + BoolToChar(state_to_set);
				m_top_level_sim_pointer->LogMessage(monitor_message);
			}
			this_pin->state = state_to_set;
			this_pin->state_changed = true;
			// Buffer output pin changes until end of Solve()...
			m_buffered_propagation = true;
		}
	} else {
		if (this_pin->name == "all_stop") {
			if (state_to_set) {
				std::string message = " ---!--- Device " + m_full_name + " ALL_STOP was asserted ---!---";
				m_top_level_sim_pointer->LogError(message);
			}
		}
	}
}

Component* Device::GetChildComponentPointer(std::string const& target_child_component_name) {
	Component* child_component_pointer = 0;
	for (const auto& this_component_descriptor : m_components) {
		if (this_component_descriptor.component_name == target_child_component_name) {
			child_component_pointer = this_component_descriptor.component_pointer;
			break;
		}
	}
	return child_component_pointer;
}

int Device::GetNestingLevel() {
	return m_nesting_level;
}

int Device::GetMessageBranchID() {
	return m_message_branch_id;
}

int Device::GetNewLocalComponentIndex() {
	int current_component_count = m_components.size();
	return current_component_count;
}

int Device::GetLocalComponentCount() {
	return m_components.size();
}

int Device::GetInPinCount() {
	int acc = 0;
	int pin_count = (int)m_pins.size();
	for (int pin_index = 0; pin_index < pin_count; pin_index ++) {
		if (m_pins[pin_index].direction == 1) {
			acc ++;
		}
	}
	return acc;
}

void Device::PrintPinStates(int max_levels) {
	if (max_levels > 0) {
		max_levels = max_levels - 1;
		std::cout << m_full_name << ": [";
		for (const auto& pin_name : GetSortedInPinNames()) {
			int pin_port_index = GetPinPortIndex(pin_name);
			if (m_pins[pin_port_index].direction == 1) {
				std::cout << " " << BoolToChar(m_pins[pin_port_index].state) << " ";
			}
		}
		std::cout << "] [";
		for (const auto& pin_name : GetSortedOutPinNames()) {
			int pin_port_index = GetPinPortIndex(pin_name);
			if (m_pins[pin_port_index].direction == 2) {
				std::cout << " " << BoolToChar(m_pins[pin_port_index].state) << " ";
			}
		}
		std::cout << "]" << std::endl;
		if (max_levels > 0) {
			PrintInternalPinStates(max_levels);
		}
	}
}

void Device::PrintInternalPinStates(int max_levels) {
	for (const auto& this_component_descriptor : m_components) {
		int this_level = max_levels;
		Component* component_pointer = this_component_descriptor.component_pointer;
		component_pointer->PrintPinStates(this_level);
	}
}

void Device::ReportUnConnectedPins() {
	if (mg_verbose_flag) {
		std::string message = "Checking pins for Device " + m_full_name + " local component id = " + std::to_string(m_local_component_index);
		m_top_level_sim_pointer->LogMessage(message);
	}
	for (const auto& this_pin : m_pins) {
		if (this_pin.direction == 1) {
			// We don't halt on a build error for un-driven input pins of upper-most level Devices.
			if ((!this_pin.drive[0]) && (m_nesting_level > 1)) {
				// Log undriven Device in pin.
				std::string build_error = "Device " + m_full_name + " in pin " + this_pin.name + " is not driven by any Component.";
				m_top_level_sim_pointer->LogError(build_error);
			}
			if (!this_pin.drive[1]) {
				// Log undriving Device in pin.
				std::string build_error = "Device " + m_full_name + " in pin " + this_pin.name + " drives no internal Components.";
				m_top_level_sim_pointer->LogError(build_error);
			}
		} else if (this_pin.direction == 2) {
			if (!this_pin.drive[0]) {
				// Log undriven Device out pin.
				std::string build_error = "Device " + m_full_name + " out pin " + this_pin.name + " is not driven by any child Component.";
				m_top_level_sim_pointer->LogError(build_error);
			}
			// We don't halt on a build error for un-driving output pins of upper-most level Devices.
			if ((!this_pin.drive[1]) && (m_nesting_level > 1)) {
				// Log undriving Device out pin.
				std::string build_error = "Device " + m_full_name + " out pin " + this_pin.name + " drives no Component.";
				m_top_level_sim_pointer->LogError(build_error);
			}
		}
	}
	int compindex = 0;
	for (const auto& this_component_descriptor : m_components) {
		this_component_descriptor.component_pointer->ReportUnConnectedPins();
		compindex ++;
	}
}

void Device::MarkInnerTerminalsDisconnected(void) {
	// As there are no conventional Components inside MagicDevice(s), if we don't mark all of the 'inner terminals' (pin.drive[1] for
	// in pins and pin.drive[0] for out pins) as 'connected', the end-of-build connections check will get upset.
	for (auto& this_pin : m_pins) {
		if (this_pin.direction == 1) {
			this_pin.drive[1] = true;
		} else if (this_pin.direction == 2) {
			this_pin.drive[0] = true;
		}
	}
}

Component* Device::SearchForComponentPointer(std::string const& target_component_full_name) {
	Component* target_component_pointer = 0;
	// Ensures that the search is run from the top-level of the Simulation, irrespective of the initial caller.
	if ((!(m_top_level_sim_pointer->GetSearchingFlag())) && (this != m_top_level_sim_pointer)) {
		m_top_level_sim_pointer->SetSearchingFlag(true);
		target_component_pointer = m_top_level_sim_pointer->SearchForComponentPointer(target_component_full_name);
	} else {
		if ((!(m_top_level_sim_pointer->GetSearchingFlag())) && (this == m_top_level_sim_pointer)) {
			m_top_level_sim_pointer->SetSearchingFlag(true);
		}
		// Search the children of this level Device.
		for (const auto& this_component_descriptor : m_components) {
			if (this_component_descriptor.component_full_name == target_component_full_name) {
				// Target Component is a child of this Device. Break and return it's pointer.
				target_component_pointer = this_component_descriptor.component_pointer;
				if (mg_verbose_flag) {
					std::cout << "Component " << target_component_full_name << " found @ " << target_component_pointer << std::endl;
				}
				break;
			}
		}
		if (target_component_pointer == 0) {
			// Target Component is not a child of this Device.
			// Search deeper into each child Device in turn. 
			for (const auto& this_device_index : m_devices) {
				Device* this_device_pointer = static_cast<Device*>(m_components[this_device_index].component_pointer);
				target_component_pointer = this_device_pointer->SearchForComponentPointer(target_component_full_name);
				// When SearchForComponentPointer() returns with a non-zero pointer, we've found the target and have it's
				// pointer, so return it. We return to the line above one level back up. In this way eventually we will
				// return the pointer to the original caller (or eventually zero, if the target does not exist).
				if (target_component_pointer != 0) {
					break;
				}
			}
		}
	}
	if ((m_top_level_sim_pointer->GetSearchingFlag()) && (this == m_top_level_sim_pointer)) {
		m_top_level_sim_pointer->SetSearchingFlag(false);
	}
	return target_component_pointer;
}

bool Device::GetDeletionFlag(void) {
	return m_deletion_flag;
}

void Device::PurgeComponent() {
	std::string header;
	if (mg_verbose_destructor_flag) {
		header =  "Purging -> DEVICE :" + m_full_name + " @ " + PointerToString(static_cast<void*>(this));
		std::cout << GenerateHeader(header) << std::endl;
	}
	// Set the Device's deletion flag to let child components know that they don't need to tidy-up
	// after themselves (remove connections from sibling components, remove themselves from Clocks,
	// Probes, etc).
	m_deletion_flag = true;
	// First  - Need to Purge and delete all child components.
	PurgeAllChildComponents();
	if (!(m_parent_device_pointer->GetDeletionFlag())) {
		// Second - Ask parent Device to purge all local references to this Device...
		// 			NOTE - Should elaborate on 'references here' really...
		//			If we are deleting this Component because we are in the process of deleting
		//			it's parent, we do not need to do this.
		m_parent_device_pointer->PurgeChildConnections(this);
	}
	if (!(m_top_level_sim_pointer->GetDeletionFlag())) {
		// Third  - Purge Device from Simulation Clocks, Probes and probable_components vector.
		//			This will 'automatically' get rid of any Probes associated with the Device
		//			(as otherwise they would target cleared memory).
		//			If we are deleting this component because we are in the process of deleting
		//			the top-level Simulation, we do not need to do this.
		m_top_level_sim_pointer->PurgeComponentFromClocks(this);
		m_top_level_sim_pointer->PurgeComponentFromProbes(this);
	}
	// Fourth - If this is a magic Device, purge it's magic Device engine.
	if (m_magic_device_flag) {
		delete m_magic_engine_pointer;
	}
	if (!(m_parent_device_pointer->GetDeletionFlag())) {
		// Fifth  - Clear component entry from parent device's m_components.
		//			If we are deleting this Component because we are in the process of deleting
		//			it's parent, we do not need to do this.
		m_parent_device_pointer->PurgeChildComponentIdentifiers(this);
	}
	if (mg_verbose_destructor_flag) {
		header =  "DEVICE : " + m_full_name + " @ " + PointerToString(static_cast<void*>(this)) + " -> Purged.";
		std::cout << GenerateHeader(header) << std::endl;
	}
	// - It should now be safe to delete this object -
}

void Device::PurgeAllChildComponents() {
	// Can't blast away at our m_components as we iterate over it, so we will make a copy on the stack and iterate over that.
	std::vector<component_descriptor> m_components_copy;
	for (const auto& this_component_descriptor : m_components) {
		component_descriptor new_component_descriptor;
		new_component_descriptor.component_name = this_component_descriptor.component_name;
		new_component_descriptor.component_full_name = this_component_descriptor.component_full_name;
		new_component_descriptor.component_pointer = this_component_descriptor.component_pointer;
		m_components_copy.push_back(new_component_descriptor);
	}
	
	// Now we can iterate over m_components_copy and blast away at m_components.
	for (const auto& copied_component_descriptor : m_components_copy) {
		delete copied_component_descriptor.component_pointer;
	}
}

void Device::PurgeChildConnections(Component* target_component_pointer) {
	// Purge connections from target Device to sibling Component in pins or parent Device out pins.
	target_component_pointer->PurgeOutboundConnections();
	// Purge connections from parent Device's in pins to target Device in pins. 
	PurgeInboundConnections(target_component_pointer);
	// Purge connections from target component's siblings out pins to target Device in pins.
	for (const auto& this_component_descriptor : m_components) {
		this_component_descriptor.component_pointer->PurgeInboundConnections(target_component_pointer);
	}
	
	// That should be all of the external
}

void Device::PurgeOutboundConnections() {
	// Purge the target's outbound connections.
	// The Components at the end of those outbound connections do not actually hold pointers to the target Component,
	// as only 'driving' pins or ports hold connection descriptors. However, the destination pins in drive flags need to be set to false.
	int port_index = 0;
	for (const auto& this_port : m_ports) {
		int pin_direction = GetPinDirection(port_index);
		if (pin_direction == 2) {		// Direction 4 hidden out pins can only be Set() from inside, so they never acquire any connection_descriptor(s) in m_ports.
			// This is an out pin so we set each target in pin drive in to false.
			for (const auto& this_connection_descriptor : this_port) {
				Component* target_component_pointer = this_connection_descriptor.target_component_pointer;
				int target_pin_direction = target_component_pointer->GetPinDirection(this_connection_descriptor.target_pin_port_index);
				std::string target_direction = "";
				if (target_pin_direction == 1) {			// Direction 3 hidden in pins can only Set() child inputs, they cannot be Set().
					target_direction = "in";
				} else if ((target_pin_direction == 2) || (target_pin_direction == 4)) {		// We will clear hidden out pin drive in flags for completeness.
					target_direction = "out";
				}
				if (mg_verbose_destructor_flag) {
					std::cout << "Component " << target_component_pointer->GetFullName() << " " << target_direction << " pin "
						<< target_component_pointer->GetPinName(this_connection_descriptor.target_pin_port_index) << " drive in set to false." << std::endl;
				}
				target_component_pointer->SetPinDrivenFlag(this_connection_descriptor.target_pin_port_index, 0, false);
			}
		}
		port_index ++;
	}
}

void Device::PurgeInboundConnections(Component* target_component_pointer) {
	// Purge m_ports (Parent *or* sibling Devices of target Component).
	// Parent Device in pins may drive out into target Component's in pins.
	// Sibling Device out pins may drive out into target Component's in pins. 
	int port_index = 0;
	std::vector<std::vector<connection_descriptor>> new_ports = {};
	for (const auto& this_port : m_ports) {
		std::vector<connection_descriptor> this_new_port = {};
		int connections_removed = 0;
		int pin_direction = GetPinDirection(port_index);
		std::string direction = "";
		// Need to catch connections to hidden pins as well, even though they do not set-off the connection checker.
		if ((pin_direction == 1) || (pin_direction == 3)) {
			direction = "in";
		} else if ((pin_direction == 2) || (pin_direction == 4)) {
			direction = "out";
		}
		for (const auto& this_connection_descriptor : this_port) {
			if (this_connection_descriptor.target_component_pointer != target_component_pointer) {
				// This connection descriptor needs to be preserved.
				this_new_port.push_back(this_connection_descriptor);
			} else {
				// This connection descriptor is to be omitted as it contains a reference to the target Component.
				connections_removed ++;
				if (mg_verbose_destructor_flag) {
					std::cout << "Device " << m_full_name << " removed an " << direction << " connection from " << GetPinName(port_index) << " to "
						<< this_connection_descriptor.target_component_pointer->GetFullName() << " in pin "
						<< this_connection_descriptor.target_component_pointer->GetPinName(this_connection_descriptor.target_pin_port_index) << std::endl;
				}
			}
		}
		new_ports.push_back(this_new_port);
		// If this port has had connection descriptors removed, and is now empty, set it's pin's drive out flag to false.
		if ((this_new_port.size() == 0) && (connections_removed > 0)) {
			if (mg_verbose_destructor_flag) {
				std::cout << "Device " << m_full_name << " " << direction << " pin " << GetPinName(port_index) << " drive out set to false."  << std::endl;
			}
			SetPinDrivenFlag(port_index, 1, false);
		}
		port_index ++;
	}
	m_ports = new_ports;
}

void Device::PurgeChildComponent(std::string const& target_component_name) {
	Component* target_component_pointer = 0;
	for (const auto& this_component_descriptor : m_components) {
		if (this_component_descriptor.component_name == target_component_name) {
			target_component_pointer = this_component_descriptor.component_pointer;
			break;
		}
	}
	if (target_component_pointer != 0) {
		delete target_component_pointer;
	} else {
		std::cout << "Child Component " << target_component_name << " not found." << std::endl;
	}
}

void Device::PurgeChildComponentIdentifiers(Component* target_component_pointer) {
	// if the Component is a child Device, parent's m_devices needs to have this Device's local component id removed.
	if (target_component_pointer->GetDeviceFlag()) {
		size_t current_local_component_index = target_component_pointer->GetLocalComponentIndex();
		std::vector<int> new_m_devices = {};
		for (size_t i = 0; i < m_devices.size(); i ++) {
			if (i != current_local_component_index) {
				new_m_devices.push_back(m_devices[i]);
			} else {
				if (mg_verbose_destructor_flag) {
					std::cout << "Omitting local component id " << i << " " << target_component_pointer->GetFullName() << " from m_devices for parent Device " << m_full_name << std::endl;
				}
			}
		}
		m_devices = new_m_devices;
		// Parent removes an entry from each of it's propagation flag vectors.
		m_propagate_this_tick_flags.pop_back();
		m_propagate_next_tick_flags.pop_back();
	}
	// Create a new m_components vector, omitting the Component to purge. 
	size_t index = 0;
	size_t removal_index = 0;
	std::vector<component_descriptor> new_m_components = {};
	for (const auto& this_component_descriptor : m_components) {
		if (this_component_descriptor.component_pointer != target_component_pointer) {
			component_descriptor new_component_descriptor;
			new_component_descriptor.component_name = this_component_descriptor.component_name;
			new_component_descriptor.component_full_name = this_component_descriptor.component_full_name;
			new_component_descriptor.component_pointer = this_component_descriptor.component_pointer;
			new_m_components.push_back(new_component_descriptor);
		} else {
			// Note the index of the purged component. 
			removal_index = index;
		}
		index ++;
	}
	m_components = new_m_components;
	// Once we have finished the new m_components list, we need to decrement m_local_component_id
	// for the components at this index and above as their position in the new m_components has shifted down one.
	for (size_t i = removal_index; i < m_components.size(); i ++) {
		component_descriptor this_component_descriptor = m_components[i];
		Component* this_component_pointer = this_component_descriptor.component_pointer;
		this_component_pointer->SetLocalComponentIndex(i);
	}
}
