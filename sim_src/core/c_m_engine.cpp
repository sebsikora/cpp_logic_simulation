/*
	
    This file is part of cpp_logic_simulation, a simple C++ framework for the simulation of digital logic circuits.
    Copyright (C) 2022 Dr Seb N.F. Sikora
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

#include "c_structs.hpp"
#include "c_m_engine.hpp"
#include "c_device.hpp"
#include "c_sim.hpp"

#include "utils.h"

MagicEngine::MagicEngine(Device* parent_device_pointer) {
	m_parent_device_pointer = parent_device_pointer;
	m_top_level_sim_pointer = m_parent_device_pointer->GetTopLevelSimPointer();
	m_identifier = m_parent_device_pointer->GetFullName() + ":magic_engine";
	m_top_level_sim_pointer->AddToMagicEngines(m_identifier, this);
}

MagicEngine::~MagicEngine() {
	std::string header;
#ifdef VERBOSE_DTORS
	header =  "Purging -> MAGICENGINE : " + m_identifier + " @ " + PointerToString(static_cast<void*>(this));
	std::cout << GenerateHeader(header) << std::endl;
#endif
	// ----------------------------------------
	//~// Then purge the magic_engine_descriptor held by the top-level Simulation.
	//~m_top_level_sim_pointer->PurgeMagicEngineDescriptorFromSimulation({m_identifier, this});
	// ----------------------------------------
#ifdef VERBOSE_DTORS
	header =  "MAGICENGINE : " + m_identifier + " @ " + PointerToString(static_cast<void*>(this)) + " -> Purged.";
	std::cout << GenerateHeader(header) << std::endl;
	std::cout << "MagicEngine dtor for " << m_identifier << " @ " << this << std::endl << std::endl;
#endif
}

void MagicEngine::AddMagicEventTrap(magic_event new_magic_event) {
	bool found = false;
	for (const auto& this_magic_event : m_magic_events) {
		if (this_magic_event == new_magic_event) {
			found = true;
			break;
		}
	}
	if (!found) {
		m_magic_events.push_back(new_magic_event);
	} else {
		std::cout << "MagicEngine " + m_identifier + " tried to add a magic event but an identical magic event already exists." << std::endl;
	}
}

void MagicEngine::CheckMagicEventTrap(int target_pin_port_index, bool new_state) {
	for (const auto& current_magic_event: m_magic_events) {
		if (target_pin_port_index == current_magic_event.target_pin_port_index) {
			bool target_pin_existing_state = m_parent_device_pointer->GetPinState(target_pin_port_index);
			std::vector<bool> current_state_change = {target_pin_existing_state, new_state};
			if (current_magic_event.state_change == current_state_change) {
				bool co_conditions_met_flag = true;
				for (const auto& current_co_condition: current_magic_event.co_conditions) {
					bool co_condition_pin_required_state = current_co_condition.pin_state;
					bool co_condition_pin_existing_state = (m_parent_device_pointer->GetPinState(current_co_condition.pin_port_index));
					co_conditions_met_flag &= (co_condition_pin_required_state == co_condition_pin_existing_state);
				}
				if (co_conditions_met_flag == true) {
					this->InvokeMagic(current_magic_event.incantation);
					break;
				}
			}
		}
	}
}
