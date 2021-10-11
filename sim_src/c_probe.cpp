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

#include "c_core.h"					// Core simulator functionality
#include "utils.h"

Probe::Probe(Simulation* top_level_device_pointer, std::string const& probe_name, std::string const& target_component_full_name, std::vector<std::string> const& target_pins, std::string const& trigger_clock_name) {
	m_top_level_sim_pointer = top_level_device_pointer;
	m_name = probe_name;
	m_target_component_full_name = target_component_full_name;
	for (const auto& pin_name: target_pins) {
		m_target_pins.push_back(std::hash<std::string>{}(pin_name));
	}
	m_trigger_clock_name = trigger_clock_name;
	m_trigger_clock_pointer = m_top_level_sim_pointer->GetClockPointer(m_trigger_clock_name);
	m_trigger_clock_pointer->AddToProbeList(m_name, this);
	m_target_component_pointer = m_top_level_sim_pointer->GetProbableComponentPointer(m_target_component_full_name);
	m_target_pin_directions = m_target_component_pointer->GetPinDirections(m_target_pins);
}

void Probe::Sample(int index) {
	m_timestamps.push_back(index);
	m_this_sample = {};
	int pin_index = 0;
	bool pin_state = false;
	for (const auto& pin_name_hash: m_target_pins) {
		int* target_pin_direction = &m_target_pin_directions[pin_index];
		if (*target_pin_direction == 1) {
			pin_state = m_target_component_pointer->GetInPinState(pin_name_hash);
		} else if (*target_pin_direction == 2) {
			pin_state = m_target_component_pointer->GetOutPinState(pin_name_hash);
		} else {
			// HELP - This terminal name corresponds to neither input or output terminal of target component.
			pin_state = false;
		}
		pin_index ++;
		m_this_sample.push_back(pin_state);
	}
	m_samples.push_back(m_this_sample);
}

void Probe::Reset() {
	m_samples.clear();
	m_timestamps.clear();
}

void Probe::PrintSamples() {
	int index = 0;
	std::cout << "Probe: " << m_name << " - " << m_target_component_full_name << std::endl;
	for (const auto& sample: m_samples) {
		std::cout << "T: " << index << "  ";
		for (const auto& sub_sample: sample) {
			if (sub_sample) {
				std::cout << " T";
			} else {
				std::cout << " F";
			}
		}
		std::cout << std::endl;
		index = index + 1;
	}
}
