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

#include "c_structs.hpp"
#include "c_clock.hpp"
#include "c_sim.hpp"
#include "c_probe.hpp"

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
#ifdef VERBOSE_DTORS
	std::cout << "Clock dtor for " << m_name << " @ " << this << std::endl << std::endl;
#endif
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
#ifdef VERBOSE_SOLVE
	std::string message = "\nT: " + std::to_string(m_index) + " " + KBLD + KYEL + "CLOCKSET: " + RST + "On tick " + KBLD + std::to_string(m_index) + RST + " " + m_name + ":clock output set to " + BoolToChar(m_toggle_pattern[m_sub_index]);
	m_top_level_sim_pointer->LogMessage(message);
#endif
	// Change output state and propagate.
	m_out_pin_state = m_toggle_pattern[m_sub_index];
	Propagate();
	m_index ++;
	m_sub_index ++;
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
			int target_pin_type = target_component_pointer->GetPinType(target_pin_port_index);
			pin::drive_state* target_pin_already_connected = target_component_pointer->CheckIfPinDriven(target_pin_port_index);
			if (!target_pin_already_connected->in) {		// Target pin drive-in must be false.
				if (target_pin_type == pin::pin_type::IN) {		// Can only connect Clock to an in pin.
					connection_descriptor new_connection_descriptor;
					new_connection_descriptor.target_component_pointer = target_component_pointer;
					new_connection_descriptor.target_pin_port_index = target_pin_port_index;
					m_connections.push_back(new_connection_descriptor);
					target_component_pointer->SetPinDrivenFlag(target_pin_port_index, pin::drive_mode::DRIVE_IN, true);
				} else {
					// Log build error here.		-- Target pin is not an in pin.
					std::string build_error = "Clock " + m_name + " tried to connect to " + target_component_name + " in pin " + target_pin_name + " but it is not an in pin.";
					m_top_level_sim_pointer->LogError(build_error);
				}
			} else {
				// Log build error here.		-- Target pin is already driven by another out pin.
				std::string build_error = "Clock " + m_name + " tried to connect to Component " + target_component_name + " in pin " + target_pin_name + " but it is already driven by another out pin.";
				m_top_level_sim_pointer->LogError(build_error);
			}
		} else {
			// Log build error here.		-- Target pin does not exist.
			std::string build_error = "Clock " + m_name + " tried to connect to Component " + target_component_name + " in pin " + target_pin_name + " but it does not exist.";
			m_top_level_sim_pointer->LogError(build_error);
		}
	} else {
		// Log build error here.		-- Component does not exist.
		std::string build_error = "Clock " + m_name + " tried to connect to Component " + target_component_name + " but it does not exist.";
		m_top_level_sim_pointer->LogError(build_error);
	}
}

void Clock::Propagate() {
	for (const auto& this_connection_descriptor : m_connections) {
		this_connection_descriptor.target_component_pointer->Set(this_connection_descriptor.target_pin_port_index, m_out_pin_state);
	}
}

bool Clock::GetTickedFlag() {
	return m_ticked_flag;
}

void Clock::TriggerProbes() {
	// Add new state to state history and then trigger all associated probes.
	if (m_probes.size() > 0) {
		m_state_history.emplace_back(m_out_pin_state);
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
#ifdef VERBOSE_DTORS
			std::cout << "Purging " << target_component_pointer->GetFullName() << " from Clock " << m_name << " m_connections." << std::endl;
#endif
		}
	}
	m_connections = new_connections;
}

void Clock::PurgeClock(void) {
	std::string header;
#ifdef VERBOSE_DTORS
	header =  "Purging -> CLOCK : " + m_name + " @ " + PointerToString(static_cast<void*>(this));
	std::cout << GenerateHeader(header) << std::endl;
#endif
	if (!(m_top_level_sim_pointer->GetDeletionFlag())) {
		// If the Clock has any connections, set their drive in flag to false.
		// If we are in the process of deleting the top-level Simulation, we do not need to do this as all connected
		// components will be deleted in any case.
		for (const auto& this_connection_descriptor : m_connections) {
			Component* target_component_pointer = this_connection_descriptor.target_component_pointer;
			int target_pin_port_index = this_connection_descriptor.target_pin_port_index;
			target_component_pointer->SetPinDrivenFlag(target_pin_port_index, pin::drive_mode::DRIVE_IN, false);
#ifdef VERBOSE_DTORS
			std::cout << "Component " << target_component_pointer->GetFullName() << " in pin " << target_component_pointer->GetPinName(target_pin_port_index) << " drive in set to false." << std::endl;
#endif
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
	}
#ifdef VERBOSE_DTORS
	header =  "CLOCK : " + m_name + " @ " + PointerToString(static_cast<void*>(this)) + " -> Purged.";
	std::cout << GenerateHeader(header) << std::endl;
#endif
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
#ifdef VERBOSE_DTORS
			std::cout << "Purging " << this_probe_descriptor.probe_name << " from Clock " << m_name << " m_probes." << std::endl;
#endif
		}
	}
	m_probes = new_probes;
}
