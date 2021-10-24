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

#include "c_core.h"					// Core simulator functionality
#include "utils.h"
#include "colors.h"

Clock::Clock(Simulation* top_level_sim_pointer, std::string const& clock_name, std::vector<bool> toggle_pattern, bool monitor_on) {
	m_top_level_sim_pointer = top_level_sim_pointer;
	m_name = clock_name;
	m_toggle_pattern = toggle_pattern;
	m_monitor_on = monitor_on;
	m_out_pin_state = m_toggle_pattern[0];
	m_index = 0;
	m_sub_index = 0;
	m_ticked_flag = false;
}

Clock::~Clock() {
	PurgeClock();
	std::cout << "Clock dtor for " << m_name << " @ " << this << std::endl << std::endl;
}

std::string Clock::GetName() {
	return m_name;
}

void Clock::AddToProbeList(std::string const& probe_identifier, Probe* probe_pointer) {
	probe_descriptor this_probe;
	this_probe.probe_name = probe_identifier;
	this_probe.probe_pointer = probe_pointer;
	m_probes.push_back(this_probe);
}

void Clock::Tick(void) {
	bool new_logical_state = m_toggle_pattern[m_sub_index];
	// Print output pin changes.
	bool verbose_output_flag = m_top_level_sim_pointer->mg_verbose_output_flag;
	if (m_monitor_on || (verbose_output_flag)) {
		std::cout << "T: " << std::to_string(m_index) << " " << BOLD(FYEL("CLOCKSET: ")) << "On tick " << BOLD("" << m_index << "") << " " << m_name << ":clock output set to " << BoolToChar(new_logical_state) << std::endl;
		if ((m_monitor_on) && !(verbose_output_flag)) {
			std::cout << std::endl;
		}
	}
	// Change output state and propagate.
	m_out_pin_state = new_logical_state;
	Propagate();
	if (verbose_output_flag) {
		std::cout << std::endl;
	}
	m_index = m_index + 1;
	m_sub_index = m_sub_index + 1;
	if (m_sub_index >= m_toggle_pattern.size()) {
		m_sub_index = 0;
	}
	m_ticked_flag = true;
}

void Clock::Reset(void) {
	m_index = 0;
	m_sub_index = 0;
	m_ticked_flag = false;
	m_state_history.clear();
	m_out_pin_state = m_toggle_pattern[0];
	for (const auto& this_probe_descriptor : m_probes) {
		this_probe_descriptor.probe_pointer->Reset();
	}
}

void Clock::Connect(std::string const& target_component_name, std::string const& target_pin_name) {
	Component* target_component_pointer = m_top_level_sim_pointer->GetChildComponentPointer(target_component_name);
	if (target_component_pointer != 0) {
		bool target_pin_exists = target_component_pointer->CheckIfPinExists(target_pin_name);
		if (target_pin_exists) {
			int target_pin_port_index = target_component_pointer->GetPinPortIndex(target_pin_name);
			int target_pin_direction = target_component_pointer->GetPinDirection(target_pin_port_index);
			std::vector<bool> target_pin_already_connected = target_component_pointer->CheckIfPinDriven(target_pin_port_index);
			if (!target_pin_already_connected[0]) {		// Target pin drive-in must be false.
				if (target_pin_direction == 1) {		// Can only connect Clock to an in pin.
					connection_descriptor new_connection_descriptor;
					new_connection_descriptor.target_component_pointer = target_component_pointer;
					new_connection_descriptor.target_pin_port_index = target_pin_port_index;
					m_connections.push_back(new_connection_descriptor);
					target_component_pointer->SetPinDrivenFlag(target_pin_port_index, false, true);
				} else {
					// Log build error here.		-- Target pin is not an in pin.
					std::string build_error = "Clock " + m_name + " tried to connect to " + target_component_name + " in pin " + target_pin_name + " but it is not an in pin.";
					m_top_level_sim_pointer->LogBuildError(build_error);
				}
			} else {
				// Log build error here.		-- Target pin is already driven by another out pin.
				std::string build_error = "Clock " + m_name + " tried to connect to Component " + target_component_name + " in pin " + target_pin_name + " but it is already driven by another out pin.";
				m_top_level_sim_pointer->LogBuildError(build_error);
			}
		} else {
			// Log build error here.		-- Target pin does not exist.
			std::string build_error = "Clock " + m_name + " tried to connect to Component " + target_component_name + " in pin " + target_pin_name + " but it does not exist.";
			m_top_level_sim_pointer->LogBuildError(build_error);
		}
	} else {
		// Log build error here.		-- Component does not exist.
		std::string build_error = "Clock " + m_name + " tried to connect to Component " + target_component_name + " but it does not exist.";
		m_top_level_sim_pointer->LogBuildError(build_error);
	}
}

void Clock::Propagate() {
	for (const auto& this_connection_descriptor : m_connections) {
		Component* target_component_pointer = this_connection_descriptor.target_component_pointer;
		target_component_pointer->Set(this_connection_descriptor.target_pin_port_index, m_out_pin_state);
	}
}

bool Clock::GetTickedFlag() {
	return m_ticked_flag;
}

void Clock::TriggerProbes() {
	// Add new state to state history and then trigger all associated probes.
	if (m_probes.size() > 0) {
		m_state_history.push_back(m_out_pin_state);
		for (const auto& this_probe_descriptor : m_probes) {
			this_probe_descriptor.probe_pointer->Sample(m_index - 1);
		}
	}
	m_ticked_flag = false;
}

void Clock::PurgeTargetComponentConnections(Component* target_component_pointer) {
	std::vector<connection_descriptor> new_connections = {};
	for (const auto& this_connection_descriptor : m_connections) {
		if (this_connection_descriptor.target_component_pointer != target_component_pointer) {
			connection_descriptor new_connection_descriptor;
			new_connection_descriptor.target_component_pointer = this_connection_descriptor.target_component_pointer;
			new_connection_descriptor.target_pin_port_index = this_connection_descriptor.target_pin_port_index;
			new_connections.push_back(new_connection_descriptor);
		} else {
			if (m_top_level_sim_pointer->mg_verbose_output_flag) {
				std::cout << "Purging " << target_component_pointer->GetFullName() << " from Clock " << m_name << " m_connections." << std::endl;
			}
		}
	}
	m_connections = new_connections;
}

void Clock::PurgeClock(void) {
	std::string header;
	if (m_top_level_sim_pointer->mg_verbose_output_flag) {
		header =  "Purging -> CLOCK : " + m_name + " @ " + PointerToString(static_cast<void*>(this));
		std::cout << GenerateHeader(header) << std::endl;
	}
	// If the Clock has any connections, set their drive in flag to false.
	for (const auto& this_connection_descriptor : m_connections) {
		Component* target_component_pointer = this_connection_descriptor.target_component_pointer;
		int target_pin_port_index = this_connection_descriptor.target_pin_port_index;
		target_component_pointer->SetPinDrivenFlag(target_pin_port_index, 0, false);
		if (m_top_level_sim_pointer->mg_verbose_output_flag) {
			std::cout << "Component " << target_component_pointer->GetFullName() << " in pin " << target_component_pointer->GetPinName(target_pin_port_index) << " drive in set to false." << std::endl;
		}
	}
	// Make a list of pointers for m_probes, then loop over *these* below. We can modify Probe.PurgeProbe() called by
	// PurgeChildProbe() below to follow the trigger clock pointer and call PurgeProbeFromClock() below to rebuild
	// this Clock's m_probes vector, omitting the target Probe.
	{
		std::vector<probe_descriptor> m_probes_copy = {};
		for (const auto& this_probe_descriptor : m_probes) {
			probe_descriptor new_probe_descriptor;
			new_probe_descriptor.probe_name = this_probe_descriptor.probe_name;
			new_probe_descriptor.probe_pointer = this_probe_descriptor.probe_pointer;
			m_probes_copy.push_back(new_probe_descriptor);
		}
		// PurgeProbe() will purge the relevant probe_descriptor from both this Clock's m_probes and the
		// parent Simulation's m_probes.
		for (const auto& this_probe_descriptor : m_probes_copy) {
			delete this_probe_descriptor.probe_pointer;
		}
		m_probes = m_probes_copy;
	}
	// Finally we purge the Clock from the parent Simulation's m_clocks vector.
	m_top_level_sim_pointer->PurgeClockDescriptorFromSimulation(this);
	if (m_top_level_sim_pointer->mg_verbose_output_flag) {
		header =  "CLOCK : " + m_name + " @ " + PointerToString(static_cast<void*>(this)) + " -> Purged.";
		std::cout << GenerateHeader(header) << std::endl;
	}
	// - It should now be safe to delete this object -
}

void Clock::PurgeProbeDescriptorFromClock(Probe* target_probe_pointer) {
	// Purge target probe's probe_descriptor from Clock's m_probes here...
	std::vector<probe_descriptor> new_probes = {};
	for (const auto& this_probe_descriptor : m_probes) {
		if (this_probe_descriptor.probe_pointer != target_probe_pointer) {
			probe_descriptor new_probe_descriptor;
			new_probe_descriptor.probe_name = this_probe_descriptor.probe_name;
			new_probe_descriptor.probe_pointer = this_probe_descriptor.probe_pointer;
			new_probes.push_back(new_probe_descriptor);
		} else {
			std::cout << "Purging " << this_probe_descriptor.probe_name << " from Clock " << m_name << " m_probes." << std::endl;
		}
	}
	m_probes = new_probes;
}
