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
#include "colors.h"

bool backwards_comp (int i, int j) {
	return (j < i);
}

Device::Device(Device* parent_device_pointer, std::string const& device_name, std::string const& device_type, std::vector<std::string> in_pin_names,
	std::vector<std::string> out_pin_names, bool monitor_on, std::unordered_map<std::string, bool> const& in_pin_default_states, int max_propagations
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
	}
	m_component_type = device_type;
	m_monitor_on = monitor_on;
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
	out_pin_names.insert(out_pin_names.end(), m_hidden_out_pins.begin(), m_hidden_out_pins.end());
	CreateOutPins(out_pin_names);
}

void Device::CreateInPins(std::vector<std::string> const& pin_names, std::unordered_map<std::string, bool> pin_default_states) {
	// Determine number of existing in and out pins.
	int new_pin_port_index = m_pins.size();
	// Create new inputs.
	for (const auto& pin_name: pin_names) {
		pin new_in_pin = {pin_name, 1, false, false, new_pin_port_index, {false, false}};
		if (!IsStringInVector(pin_name, m_hidden_in_pins)) {
			// If this is a user-defined input, handle as normal.
			if (IsStringInMapKeys(pin_name, pin_default_states)) {
				// If this input is in the defaults list set accordingly...
				new_in_pin.state = pin_default_states[pin_name];
			} else {
				// ...otherwise set input state to false.
				new_in_pin.state = false;
			}
		} else {
			// Set hidden in pin default states.
			new_in_pin.direction = 3;
			if (pin_name == "true") {
				new_in_pin.state = true;
			} else if (pin_name == "false") {
				new_in_pin.state = false;
			}
		}
		new_in_pin.state_changed = false;
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
		if (IsStringInVector(pin_name, m_hidden_out_pins)) {
			new_out_pin.direction = 4;
		}
		m_pins.push_back(new_out_pin);
		m_ports.push_back({});
		new_pin_port_index ++;
	}
}

void Device::Build() {
	// Redefined for each specific device subclass...
}

void Device::Stabilise() {
	// Ensures that internal device state settles correctly.
	if (this == m_top_level_sim_pointer) {
		ReportUnConnectedPins();
	}
	if (mg_verbose_output_flag) {
		std::string msg = "Stabilising new level " + std::to_string(m_nesting_level) + " device " + m_full_name;
		std::cout << GenerateHeader(msg) << std::endl << std::endl;
	}
	// First we Set() all child component inputs that are connected to the parent device inputs to deafult states.
	for (const auto& this_pin : m_pins) {
		if ((this_pin.direction == 1) || (this_pin.direction == 3)) {
			for (const auto& this_connection_descriptor : m_ports[this_pin.port_index]) {
				// The same idiom is used to go from an entry in m_ports to setting the appropriate target pin, in the
				// Propagate() method further down.
				Component* target_component_pointer = this_connection_descriptor.target_component_pointer;
				int target_pin_port_index = this_connection_descriptor.target_pin_port_index;
				target_component_pointer->Set(target_pin_port_index, this_pin.state);
			}
		}
	}
	if (mg_verbose_output_flag) {
		std::cout << std::endl;
	}
	// Next we call Initialise() for all Components.
	for (const auto& this_component_descriptor : m_components) {
			if (mg_verbose_output_flag) {
				std::cout << "Initialising " << this_component_descriptor.component_full_name << std::endl;
			}
			this_component_descriptor.component_pointer->Initialise();
	}
	if (mg_verbose_output_flag) {
		std::cout << std::endl;
	}
	// Lastly we Solve() by iterating SubTick() until device internal state has stabilised.
	// In the runtime Solve() loop, we need to handle the m_buffered_propagations flag returned
	// by Solve(). We don't need to do that here, as the parent Device will add *all* it's child
	// Devices to it's m_propagate_next list, as we did here above.
	Solve();
	if (mg_verbose_output_flag) {
		std::cout << GenerateHeader("Starting state settled.") << std::endl << std::endl;
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
	m_parent_device_pointer->AddToPropagateNextTick(m_local_component_index);
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
		m_devices.push_back(new_component_pointer->m_local_component_index);
	}
}

void Device::AddGate(std::string const& component_name, std::string const& component_type, std::vector<std::string> const& in_pin_names, bool monitor_on) {
	AddComponent(new Gate(this, component_name, component_type, in_pin_names, monitor_on));
}

void Device::AddGate(std::string const& component_name, std::string const& component_type, bool monitor_on) {
	AddComponent(new Gate(this, component_name, component_type, {}, monitor_on));
}

void Device::AddGate(std::string const& component_name, std::string const& component_type) {
	AddComponent(new Gate(this, component_name, component_type, {}, false));
}

void Device::AddMagicEventTrap(std::string const& target_pin_name, std::vector<bool> const& state_change, std::vector<human_writable_magic_event_co_condition> const& hw_co_conditions, std::string const& incantation) {
	if (m_magic_device_flag == true) {
		// Convert the human-writable  kind of magic event co-condition (strings for terminal identifiers) to the kind pin-indexed kind.
		std::vector<magic_event_co_condition> co_conditions = {};
		for (int i = 0; i  < hw_co_conditions.size(); i ++) {
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
		std::string identifier = target_pin_name + ":" + incantation;
		m_magic_engine_pointer->AddMagicEventTrap(identifier, new_magic_event);
	} else {
		std::cout << "Device " + m_full_name + " is not magic! Cannot add magic event trap." << std::endl;
	}
}

void Device::ChildConnect(std::string const& target_child_component_name, std::vector<std::string> const& connection_parameters) {
	Component* target_component_pointer = GetChildComponentPointer(target_child_component_name);
	if (target_component_pointer == 0) {
		// Log build error here.		-- Child Component does not exist.
		std::string build_error = "Device " + m_full_name + " tried to connect from child Component " + target_child_component_name + " but it does not exist.";
		m_top_level_sim_pointer->LogBuildError(build_error);
	} else {
		target_component_pointer->Connect(connection_parameters);
	}
}

void Device::ChildSet(std::string const& target_child_component_name, std::string const& target_pin_name, bool logical_state) {
	Component* target_component_pointer = GetChildComponentPointer(target_child_component_name);
	if (target_component_pointer == 0) {
		// Log build error here.		-- Child Component does not exist.
		std::string build_error = "Device " + m_full_name + " tried to Set() an input of child Component " + target_child_component_name + " but it does not exist.";
		m_top_level_sim_pointer->LogBuildError(build_error);
	} else {
		int target_pin_port_index = target_component_pointer->GetPinPortIndex(target_pin_name);
		if (mg_verbose_output_flag || target_component_pointer->m_monitor_on) {
			std::cout << BOLD(FYEL("CHILDSET: ")) << "Component " << BOLD("" << target_component_pointer->GetFullName() << ":" << target_component_pointer->GetComponentType() << "") << " terminal " << BOLD("" << target_pin_name << "") << " set to " << BoolToChar(logical_state) << std::endl;
		}
		target_component_pointer->Set(target_pin_port_index, logical_state);
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
}

void Device::ChildPrintPinStates(std::string const& target_child_component_name, int max_levels) {
	Component* target_component_pointer = GetChildComponentPointer(target_child_component_name);
	if (target_component_pointer == 0) {
		// Log build error here.		-- Child Component does not exist.
		std::string build_error = "Device " + m_full_name + " tried to recursively print pin states from child Component " + target_child_component_name + " but it does not exist.";
		m_top_level_sim_pointer->LogBuildError(build_error);
	} else {
		target_component_pointer->PrintPinStates(max_levels);
	}
}

void Device::ChildPrintInPinStates(std::string const& target_child_component_name) {
	Component* target_component_pointer = GetChildComponentPointer(target_child_component_name);
	if (target_component_pointer == 0) {
		// Log build error here.		-- Child Component does not exist.
		std::string build_error = "Device " + m_full_name + " tried to print in pin states for child Component " + target_child_component_name + " but it does not exist.";
		m_top_level_sim_pointer->LogBuildError(build_error);
	} else {
		target_component_pointer->PrintInPinStates();
	}
}

void Device::ChildPrintOutPinStates(std::string const& target_child_component_name) {
	Component* target_component_pointer = GetChildComponentPointer(target_child_component_name);
	if (target_component_pointer == 0) {
		// Log build error here.		-- Child Component does not exist.
		std::string build_error = "Device " + m_full_name + " tried to print out pin states for child Component " + target_child_component_name + " but it does not exist.";
		m_top_level_sim_pointer->LogBuildError(build_error);
	} else {
		target_component_pointer->PrintOutPinStates();
	}
}

void Device::ChildMakeProbable(std::string const& target_child_component_name) {
	Component* target_component_pointer = GetChildComponentPointer(target_child_component_name);
	if (target_component_pointer == 0) {
		// Log build error here.		-- Child Component does not exist.
		std::string build_error = "Device " + m_full_name + " tried to make child Component " + target_child_component_name + " probable but it does not exist.";
		m_top_level_sim_pointer->LogBuildError(build_error);
	} else {
		target_component_pointer->MakeProbable();
	}
}

void Device::ChildMarkOutputNotConnected(std::string const& target_child_component_name, std::string const& target_out_pin_name) {
	Component* target_component_pointer = GetChildComponentPointer(target_child_component_name);
	if (target_component_pointer == 0) {
		// Log build error here.		-- Child Component does not exist.
		std::string build_error = "Device " + m_full_name + " tried to mark child Component " + target_child_component_name + " output pin " + target_out_pin_name + " not connected but the component does not exist.";
		m_top_level_sim_pointer->LogBuildError(build_error);
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
			m_top_level_sim_pointer->LogBuildError(build_error);
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
			Component* target_component_pointer;
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
				m_top_level_sim_pointer->LogBuildError(build_error);
			} else {
				if (!target_pin_exists) {
					// Log build error here.		-- Target pin does not exist.
					std::string build_error = "Device " + m_full_name + " tried to connect " + origin_direction + " " + origin_pin_name + " to " + target_component_nature_string + " component " + target_component_name + " pin " + target_pin_name + " but it does not exist.";
					m_top_level_sim_pointer->LogBuildError(build_error);
				} else {
					if (!target_pin_direction_compatible) {
						// Log build error here.		-- Origin and target pin directions are not compatible.
						std::string build_error = "Device " + m_full_name + " tried to connect " + origin_direction + " " + origin_pin_name + " to " + target_component_nature_string + " component " + target_component_name + " " + target_direction + " pin " + target_pin_name + " but they are not compatible";
						m_top_level_sim_pointer->LogBuildError(build_error);
					} else {
						if (!no_existing_connection) {
							// Log build error here.		-- This connection already exists.
							std::string build_error = "Device " + m_full_name + " tried to connect " + origin_direction + " " + origin_pin_name + " to " + target_component_nature_string + " component " + target_component_name + " pin " + target_pin_name + " but is already connected to it.";
							m_top_level_sim_pointer->LogBuildError(build_error);
						} else {
							if (target_pin_already_driven[0]) {
								// Log build error here.		-- This target pin is already driven by another pin.
								std::string build_error = "Device " + m_full_name + " tried to connect " + origin_direction + " " + origin_pin_name + " to " + target_component_nature_string + " component " + target_component_name + " pin " + target_pin_name + " but it is already driven by another pin.";
								m_top_level_sim_pointer->LogBuildError(build_error);
							}
						}
					}
				}
			}
		} else {
			// Log build error here.		-- Origin pin does not exist.
			std::string build_error = "Device " + m_full_name + " tried to connect from input " + origin_pin_name + " but it does not exist.";
			m_top_level_sim_pointer->LogBuildError(build_error);
		}
	} else {
		// Log build error here.		-- Wrong number of connection parameters.
		std::string build_error = "Device " + m_full_name + " tried to form a connection but the wrong number of connection parameters were provided.";
		m_top_level_sim_pointer->LogBuildError(build_error);
	}
}

bool Device::CheckAndClearSolutionFlag() {
	bool current_solution_flag = m_solve_this_tick_flag;
	m_solve_this_tick_flag = false;
	return current_solution_flag;
}

bool Device::Solve() {
	int sub_tick_count = 0;
	int sub_tick_limit = m_max_propagations;
	while (m_propagate_next_tick.size() > 0) {
		if (mg_verbose_output_flag) {
			std::cout << BOLD(FMAG("Level " << BOLD("" << m_nesting_level << "") << " Device " << BOLD("" << m_full_name << "") << " starting to Solve()...")) << std::endl << std::endl;
		}
		// Handle pending propagations for child Gates and child Device in pins.
		// Loop terminates when there are no outstanding propagations.
		while (sub_tick_count <= sub_tick_limit) {
			SubTick(sub_tick_count);
			if (m_propagate_next_tick.size() > 0) {
				sub_tick_count += 1;
			} else {
				break;
			}
		}
		if (sub_tick_count > sub_tick_limit) {
			//~// Log error here.		-- Not able to stabilise Device state.
			//~std::string build_error = "~~~~~~~~";
			//~m_top_level_sim_pointer->LogBuildError(build_error);
			std::cout << "Could not stabilise " << m_full_name << " state within " << m_max_propagations << " propagation steps." << std::endl;
			break;
		} else {
			if (mg_verbose_output_flag) {
				std::cout << "...Solve()d." << std::endl << std::endl;
			}
		}
		// Solve() for all pending child Devices.
		for (const auto& this_local_device_index : m_devices) {
			Device* this_device_pointer = static_cast<Device*>(m_components[this_local_device_index].component_pointer);
			if (this_device_pointer->CheckAndClearSolutionFlag()) {
				if ((this_device_pointer->m_monitor_on) && !(mg_verbose_output_flag)) {
					std::cout << std::endl;
				}
				// If the child Device's Solve() returns true, one or more of it's out pins have
				// changed and it needs to be added to the propagation list.
				if (this_device_pointer->Solve()) {
					m_propagate_next_tick.emplace_back(this_local_device_index);
				}
			}
		}
	}
	// If we're solving the top-level Simulation state, we need to check if the Clock has triggered any Probes.
	if (this == m_top_level_sim_pointer) {
		m_top_level_sim_pointer->CheckProbeTriggers();
	}
	// Set m_buffered_propagation to false and return it's previous value.
	bool buffered_propagation_to_return = m_buffered_propagation;
	m_buffered_propagation = false;
	return buffered_propagation_to_return;
}

void Device::SubTick(int index) {
	if (mg_verbose_output_flag) {
		std::cout << "Iteration: " << std::to_string(index) << std::endl;
	}
	// Propagation queue for the next subtick can be set up in one of two ways:
	// If the component local IDs are sorted in ascending order, we need to delete the 0th entry each time in the
	// propagation loop that follows. It's faster to pop_back() the last element instead, but to do that we need
	// to inverse-sort the component local IDs and then work through them from last to first.
	// This also requires that we modify std::binary_search in CheckIfQueuedToPropagateThisTick() for searching
	// an inverse-sorted vector. We use the same custom < comparator function (backwards_comp() defined at the top)
	// to modify the behaviour of both std::sort() and std::binary_search().
	// To compare:
	// Swap the commented and uncommented std::sort(s) below this, and then swap the commented and uncommented for-loops.
	// Also swap the commented and uncommented std::binary_search(es) in CheckIfQueuedToPropagateThisTick().
	//~std::sort(m_propagate_next_tick.begin(), m_propagate_next_tick.end(), backwards_comp);
	// 		NOTE - Confusingly the forward sort is uncommented here at the moment as the following for-loop copies
	// 		m_propagate_next_tick *backwards* so it can clear it by popping elements off the end, as this seems yet-faster.
	std::sort(m_propagate_next_tick.begin(), m_propagate_next_tick.end());
	m_propagate_next_tick.erase(std::unique(m_propagate_next_tick.begin(), m_propagate_next_tick.end()), m_propagate_next_tick.end()); 
	int copy_range = m_propagate_next_tick.size();
	for (int i = copy_range - 1; i >= 0; i --) {
		m_propagate_this_tick.emplace_back(m_propagate_next_tick[i]);
		m_still_to_propagate.emplace_back(m_propagate_next_tick[i]);
		m_propagate_next_tick.pop_back();
	}
	//~for (const auto& this_component_local_index : m_propagate_this_tick) {
		//~m_still_to_propagate.erase(m_still_to_propagate.begin());
		//~Component* component_pointer = m_components[this_component_local_index].component_pointer;
		//~component_pointer->Propagate();
	//~}
	for (int i = m_propagate_this_tick.size() - 1; i >= 0; i --) {
		m_still_to_propagate.pop_back();
		m_components[m_propagate_this_tick[i]].component_pointer->Propagate();
	}
	m_propagate_this_tick.clear();
	if (mg_verbose_output_flag) {
		std::cout << std::endl;
	}
}

void Device::Propagate() {
	for (auto& this_pin : m_pins) {
		if (this_pin.state_changed) {
			if (mg_verbose_output_flag) {
				std::string direction_string = "";
				if (this_pin.direction == 1) {
					direction_string = "input ";
				} else {
					direction_string = "output ";
				}
				std::cout << BOLD(FGRN("->")) << " Device " << BOLD("" << m_full_name << "") << " propagating " << direction_string << this_pin.name << " = " << BoolToChar(this_pin.state) << std::endl;
			}
			this_pin.state_changed = false;
			for (const auto& this_connection_descriptor : m_ports[this_pin.port_index]) {
				this_connection_descriptor.target_component_pointer->Set(this_connection_descriptor.target_pin_port_index, this_pin.state);
			}
		}
	}
}

void Device::Set(int pin_port_index, bool state_to_set) {
	pin* this_pin = &m_pins[pin_port_index];
	if (this_pin->direction == 1) {
		if (state_to_set != this_pin->state) {
			if ((m_monitor_on) || (mg_verbose_output_flag)) {
				if (mg_verbose_output_flag) {
					std::cout << BOLD(FGRN("  ->")) << " Device " << BOLD("" << m_full_name << "") << " input terminal " << BOLD("" << this_pin->name << "") << " set from " << BoolToChar(this_pin->state) << " to " << BoolToChar(state_to_set) << std::endl;
				}
				std::cout << BOLD(FRED("  MONITOR: ")) << "Component " << BOLD("" << m_full_name << ":" << m_component_type << " ") << "input terminal " << BOLD("" << this_pin->name << "") << " set to " << BoolToChar(state_to_set) << std::endl;
			}
			if (m_magic_device_flag == true) {
				m_magic_engine_pointer->CheckMagicEventTrap(pin_port_index, state_to_set);
			}
			this_pin->state = state_to_set;
			this_pin->state_changed = true;
			// Add device to the parent Devices propagate_next list, UNLESS this device
			// is already queued-up to propagate this SubTick.
			if (m_parent_device_pointer->CheckIfQueuedToPropagateThisTick(m_local_component_index) == false) {
				m_parent_device_pointer->AddToPropagateNextTick(m_local_component_index);
			}
			m_solve_this_tick_flag = true;
		}
	} else if (this_pin->direction == 2) {
		if (state_to_set != this_pin->state) {
			if ((m_monitor_on) || (mg_verbose_output_flag)) {
				if (mg_verbose_output_flag) {
					std::cout << BOLD(FGRN("  ->")) << " Device " << BOLD("" << m_full_name << "") << " output terminal " << BOLD("" << this_pin->name << "") << " set from " << BoolToChar(this_pin->state) << " to " << BoolToChar(state_to_set) << std::endl;
				}
				std::cout << BOLD(FRED("  MONITOR: ")) << "Component " << BOLD("" << m_full_name << ":" << m_component_type << " ") << "output terminal " << BOLD("" << this_pin->name << "") << " set to " << BoolToChar(state_to_set) << std::endl;
			}
			this_pin->state = state_to_set;
			this_pin->state_changed = true;
			// Buffer output pin changes until end of Solve()...
			m_buffered_propagation = true;
		}
	} else {
		if ((this_pin->name == "all_stop") && (m_top_level_sim_pointer->IsSimulationRunning())) {
			if (state_to_set) {
				std::cout << BOLD(" ---!--- Device " << m_full_name << " ALL_STOP was asserted ---!---") << std::endl;
				m_top_level_sim_pointer->StopSimulation();
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

int Device::GetNewLocalComponentIndex() {
	int current_component_count = m_components.size();
	return current_component_count;
}

bool Device::CheckIfQueuedToPropagateThisTick(int propagation_identifier) {
	return std::binary_search(m_still_to_propagate.begin(), m_still_to_propagate.end(), propagation_identifier, backwards_comp);
	//~return std::binary_search(m_still_to_propagate.begin(), m_still_to_propagate.end(), propagation_identifier);
}

void Device::AddToPropagateNextTick(int propagation_identifier) {
	m_propagate_next_tick.emplace_back(propagation_identifier);
}

void Device::MakeProbable() {
	if (this != m_top_level_sim_pointer) {
		m_top_level_sim_pointer->AddToProbableComponents(this);
	}
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
	for (const auto& this_pin : m_pins) {
		if (this_pin.direction == 1) {
			// We don't halt on a build error for un-driven input pins of upper-most level Devices.
			if ((!this_pin.drive[0]) && (m_nesting_level > 1)) {
				// Log undriven Device in pin.
				std::string build_error = "Device " + m_full_name + " in pin " + this_pin.name + " is not driven by any Component.";
				m_top_level_sim_pointer->LogBuildError(build_error);
			}
			if (!this_pin.drive[1]) {
				// Log undriving Device in pin.
				std::string build_error = "Device " + m_full_name + " in pin " + this_pin.name + " drives no internal Components.";
				m_top_level_sim_pointer->LogBuildError(build_error);
			}
		} else if (this_pin.direction == 2) {
			if (!this_pin.drive[0]) {
				// Log undriven Device out pin.
				std::string build_error = "Device " + m_full_name + " out pin " + this_pin.name + " is not driven by any child Component.";
				m_top_level_sim_pointer->LogBuildError(build_error);
			}
			// We don't halt on a build error for un-driving output pins of upper-most level Devices.
			if ((!this_pin.drive[1]) && (m_nesting_level > 1)) {
				// Log undriving Device out pin.
				std::string build_error = "Device " + m_full_name + " out pin " + this_pin.name + " drives no Component.";
				m_top_level_sim_pointer->LogBuildError(build_error);
			}
		}
	}
	for (const auto& this_component_descriptor : m_components) {
		this_component_descriptor.component_pointer->ReportUnConnectedPins();
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
	for (const auto& this_component_descriptor : m_components) {
		if (this_component_descriptor.component_full_name == target_component_full_name) {
			// Target Component is a child of this Device. Break and return it's pointer.
			target_component_pointer = this_component_descriptor.component_pointer;
			std::cout << "Component " << target_component_full_name << " found @ " << target_component_pointer << std::endl;
			break;
		}
	}
	if (target_component_pointer == 0) {
		// Target Component is not a child of this Device.
		// Call PurgeComponent() for each child Device to search deeper.
		for (const auto& this_device_index : m_devices) {
			Device* this_device_pointer = static_cast<Device*>(m_components[this_device_index].component_pointer);
			target_component_pointer = this_device_pointer->SearchForComponentPointer(target_component_full_name);
			// When PurgeComponent() returns with a non-zero pointer, break and return it.
			// In this way eventually we will return the pointer to the original caller.
			if (target_component_pointer != 0) {
				break;
			}
		}
	}
	return target_component_pointer;
}

void Device::PurgeComponent() {
	// Ask parent device to purge all local references to this Device...
	m_parent_device_pointer->PurgeChildConnections(this);
}

void Device::PurgeInboundConnections(Component* target_component_pointer) {
	// Purge m_ports of connections to or from target Component.
	int port_index = 0;
	std::vector<std::vector<connection_descriptor>> new_ports = {};
	for (const auto& this_port : m_ports) {
		std::vector<connection_descriptor> this_new_port = {};
		int connections_removed = 0;
		int pin_direction = GetPinDirection(port_index);
		std::string direction = "";
		if (pin_direction == 1) {
			direction = "in";
		} else if (pin_direction == 2) {
			direction = "out";
		}
		for (const auto& this_connection_descriptor : this_port) {
			if (this_connection_descriptor.target_component_pointer != target_component_pointer) {
				// This connection descriptor needs to be preserved.
				this_new_port.push_back(this_connection_descriptor);
			} else {
				// This connection descriptor is to be omitted and any corresponding drive flags set to false.
				connections_removed ++;
				std::cout << "Device " << m_full_name << " removed an " << direction << " connection from " << GetPinName(port_index) << " to "
					<< this_connection_descriptor.target_component_pointer->GetFullName() << " in pin "
					<< this_connection_descriptor.target_component_pointer->GetPinName(this_connection_descriptor.target_pin_port_index) << std::endl;
			}
		}
		new_ports.push_back(this_new_port);
		if ((this_new_port.size() == 0) && (connections_removed > 0)) {
			std::cout << "Device " << m_full_name << " " << direction << " pin " << GetPinName(port_index) << " drive out set to false."  << std::endl;
			SetPinDrivenFlag(port_index, 1, false);
		}
		port_index ++;
	}
	m_ports = new_ports;
}

void Device::PurgeOutboundConnections() {
	int port_index = 0;
	for (const auto& this_port : m_ports) {
		int pin_direction = GetPinDirection(port_index);
		if (pin_direction == 2) {
			// This is an out pin so we set each target in pin drive in to false.
			for (const auto& this_connection_descriptor : this_port) {
				Component* target_component_pointer = this_connection_descriptor.target_component_pointer;
				int target_pin_direction = target_component_pointer->GetPinDirection(this_connection_descriptor.target_pin_port_index);
				std::string target_direction = "";
				if (target_pin_direction == 1) {
					target_direction = "in";
				} else if (target_pin_direction == 2) {
					target_direction = "out";
				}
				std::cout << "Component " << target_component_pointer->GetFullName() << " " << target_direction << " pin "
					<< target_component_pointer->GetPinName(this_connection_descriptor.target_pin_port_index) << " drive in set to false." << std::endl;
				target_component_pointer->SetPinDrivenFlag(this_connection_descriptor.target_pin_port_index, 0, false);
			}
		}
		port_index ++;
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
}
