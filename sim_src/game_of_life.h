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

#ifndef LSIM_GAME_OF_LIFE_H
#define LSIM_GAME_OF_LIFE_H

#include <string>					// std::string.
#include <vector>					// std::vector

#include "c_core.h"					// Core simulator functionality

class GameOfLife_Cell_Decider : public Device {
	public:
		GameOfLife_Cell_Decider(Device* parent_device_pointer, std::string name, bool monitor_on = false, std::vector<state_descriptor> in_pin_default_states = {});
		void Build(void);
};

class GameOfLife_Cell_SiblingSelector : public Device {
	public:
		GameOfLife_Cell_SiblingSelector(Device* parent_device_pointer, std::string name, bool monitor_on = false, std::vector<state_descriptor> in_pin_default_states = {});
		void Build(void);
};

class GameOfLife_Cell : public Device {
	public:
		GameOfLife_Cell(Device* parent_device_pointer, std::string name, bool monitor_on = false, std::vector<state_descriptor> in_pin_default_states = {});
		void Build(void);
};

#endif
