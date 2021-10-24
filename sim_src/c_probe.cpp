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

Probe::Probe(Simulation* top_level_sim_pointer, std::string const& probe_name, Component* target_component_pointer,
	std::vector<std::string> const& target_pin_names, Clock* trigger_clock_pointer
	) {
	// Probe constructor is called from within top-level Simulation, constructor arguments are all sanity-checked there.
	m_top_level_sim_pointer = top_level_sim_pointer;
	m_target_component_pointer = target_component_pointer;
	m_name = probe_name;
	m_target_component_full_name = target_component_pointer->GetFullName();
	m_trigger_clock_name = trigger_clock_pointer->GetName();
	m_trigger_clock_pointer = trigger_clock_pointer;
	m_trigger_clock_pointer->AddToProbeList(m_name, this);
	for (const auto& pin_name : target_pin_names) {
		m_target_pin_indices.push_back(m_target_component_pointer->GetPinPortIndex(pin_name));
	}
}

void Probe::Sample(int index) {
	m_this_sample.clear();
	m_timestamps.emplace_back(index);
	bool pin_state = false;
	for (const auto& pin_port_index : m_target_pin_indices) {
		pin_state = m_target_component_pointer->GetPinState(pin_port_index);
		m_this_sample.emplace_back(pin_state);
	}
	m_samples.emplace_back(m_this_sample);
}

void Probe::Reset() {
	m_this_sample.clear();
	m_samples.clear();
	m_timestamps.clear();
}

void Probe::PreallocateSampleMemory(int number_of_ticks) {
	size_t current_size = m_samples.size();
	size_t new_capacity = current_size + number_of_ticks;
	m_samples.reserve(new_capacity);
	m_timestamps.reserve(new_capacity);
	m_this_sample.reserve(m_target_pin_indices.size());
	
}

Component* Probe::GetTargetComponentPointer() {
	return m_target_component_pointer;
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

std::vector<std::vector<bool>> Probe::GetSamples(void) {
	return m_samples;
}

void Probe::PurgeProbe() {
	std::string header;
	if (m_top_level_sim_pointer->mg_verbose_output_flag) {
		header =  "Purging -> PROBE : " + m_name + " @ " + PointerToString(static_cast<void*>(this)) ;
		std::cout << std::endl << GenerateHeader(header) << std::endl << std::endl;
	}
	// PurgeProbe needs to follow the m_trigger_clock_pointer to first of all remove itself from the trigger Clock's m_probes vector...
	m_trigger_clock_pointer->PurgeProbeDescriptorFromClock(this);
	// ...then parent Simulation's m_probes vector.
	m_top_level_sim_pointer->PurgeProbeDescriptorFromSimulation(this);
	if (m_top_level_sim_pointer->mg_verbose_output_flag) {
		header =  "PROBE : " + m_name + " @ " + PointerToString(static_cast<void*>(this)) + " -> Purged." ;
		std::cout << std::endl << GenerateHeader(header) << std::endl << std::endl;
	}
	// - It should now be safe to delete this object -
}
