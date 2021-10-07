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

MagicEngine::MagicEngine(Device* parent_device_pointer) {
	m_parent_device_pointer = parent_device_pointer;
	m_top_level_sim_pointer = m_parent_device_pointer->GetTopLevelSimPointer();
	std::string identifier = m_parent_device_pointer->GetFullName() + ":magic_engine";
	m_top_level_sim_pointer->AddToMagicEngines(identifier, this);
}

void MagicEngine::AddMagicEventTrap(std::string identifier, magic_event new_magic_event) {
	std::size_t identifier_hash = std::hash<std::string>{}(identifier);
	if (IsStringInMapKeys(identifier, m_magic_events) == false) {
		m_magic_events[identifier] = new_magic_event;
	}
}

void MagicEngine::InvokeMagic(std::string const& incantation) {
	// Redefined for each specific device subclass...
}

void MagicEngine::UpdateMagic() {
	// Redefined for each specific device subclass...
}

void MagicEngine::ShutDownMagic() {
	// Redefined for each specific device subclass...
}

void MagicEngine::CheckMagicEventTrap(std::size_t target_pin_name_hash, bool new_state) {
	for (const auto& current_event_trap: m_magic_events) {
		magic_event current_magic_event = current_event_trap.second;
		if (target_pin_name_hash == current_magic_event.target_pin_name_hash) {
			bool target_pin_existing_state = m_parent_device_pointer->GetInPinState(target_pin_name_hash);
			std::vector<bool> current_state_change = {target_pin_existing_state, new_state};
			if (current_magic_event.state_change == current_state_change) {
				bool co_conditions_met_flag = true;
				for (const auto& current_co_condition: current_magic_event.co_conditions) {
					bool co_condition_pin_required_state = current_co_condition.pin_state;
					bool co_condition_pin_existing_state = (m_parent_device_pointer->GetInPinState(current_co_condition.pin_name_hash));
					co_conditions_met_flag &= (co_condition_pin_required_state == co_condition_pin_existing_state);
				}
				if (co_conditions_met_flag == true) {
					this->InvokeMagic(current_magic_event.incantation);
				}
			}
		}
	}
}
