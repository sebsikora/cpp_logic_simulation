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

#ifndef LSIM_CORE_MAGIC_ENG_HPP
#define LSIM_CORE_MAGIC_ENG_HPP

#include <string>
#include <vector>

#include "c_structs.hpp"

class Device;
class Simulation;

class MagicEngine {
	public:
		MagicEngine(Device* parent_device_pointer);
		virtual ~MagicEngine();
		
		virtual void InvokeMagic(int incantation) = 0;
		virtual void UpdateMagic(void) = 0;
		virtual void ShutDownMagic(void) = 0;

		void AddMagicEventTrap(magic_event new_magic_event);
		void CheckMagicEventTrap(int target_pin_port_index, bool new_state);
		
		Device* m_parent_device_pointer;
		Simulation* m_top_level_sim_pointer;
	
	protected:
		std::string m_identifier;
	
	private:
		std::vector<magic_event> m_magic_events;
};

#endif	// LSIM_CORE_MAGIC_ENG_HPP
