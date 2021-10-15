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

#include "c_core.h"					// Core simulator functionality
#include "utils.h"
#include "colors.h"

Clock::Clock(Device* parent_device_pointer, std::string const& clock_name, std::vector<bool> toggle_pattern, bool monitor_on) {
	m_parent_device_pointer = parent_device_pointer;
	m_name = clock_name;
	m_name_hash = std::hash<std::string>{}(m_name);
	m_toggle_pattern = toggle_pattern;
	m_monitor_on = monitor_on;
	m_out_pin_state = m_toggle_pattern[0];
	m_index = 0;
	m_sub_index = 0;
	m_ticked_flag = false;
}

void Clock::AddToProbeList(std::string const& probe_identifier, Probe* probe_pointer) {
	bool probe_exists = IsStringInMapKeys(probe_identifier, m_probes);
	if (probe_exists == false) {
		m_probes[probe_identifier] = probe_pointer;
	}
}

void Clock::Tick(void) {
	bool new_logical_state = m_toggle_pattern[m_sub_index];
	// Print output pin changes.
	bool verbose_output_flag = m_parent_device_pointer->mg_verbose_output_flag;
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
	for (const auto& current_probe: m_probes) {
		current_probe.second->Reset();
	}
}

void Clock::Connect(std::string const& target_component_name, std::string const& pin_name) {
	Component* target_component;
	if (target_component_name == "parent") {
		target_component = m_parent_device_pointer;
	} else {
		target_component = m_parent_device_pointer->GetChildComponentPointer(target_component_name);
	}
	connection_descriptor conn_descriptor;
	conn_descriptor.target_component = target_component;
	std::size_t pin_name_hash = std::hash<std::string>{}(pin_name);
	conn_descriptor.target_pin_name_hash = pin_name_hash;
	conn_descriptor.target_pin_direction = target_component->GetPinDirection(pin_name_hash);
	m_connections.push_back(conn_descriptor);
}

void Clock::Propagate() {
	for (const auto& connection: m_connections) {
		connection_descriptor target_connection_descriptor = connection;
		Component* target_component = target_connection_descriptor.target_component;
		target_component->Set(target_connection_descriptor.target_pin_name_hash, target_connection_descriptor.target_pin_direction, m_out_pin_state);
	}
}

bool Clock::GetTickedFlag() {
	return m_ticked_flag;
}

void Clock::TriggerProbes() {
	// Add new state to state history and then trigger all associated probes.
	if (m_probes.size() > 0) {
		m_state_history.push_back(m_out_pin_state);
		for (const auto& current_probe: m_probes) {
			current_probe.second->Sample(m_index - 1);
		}
	}
	m_ticked_flag = false;
}
