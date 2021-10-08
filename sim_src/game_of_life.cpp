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
#include "game_of_life.h"

GameOfLife_Cell_Brain::GameOfLife_Cell_Brain(Device* parent_device_pointer, std::string cell_name, bool monitor_on, std::unordered_map<std::string, bool> in_pin_default_states) 
 : Device(parent_device_pointer, cell_name, "g_o_l_cell", {"count_in_0", "count_in_1", "count_in_2", "alive"}, {"alive"}, monitor_on, in_pin_default_states) {
	 // Following base class constructor (Device), we call the below overridden Build() method to populate the
	 // specific device, then we call the base Stabilise() method to configure initial internal device component state.
	 Build();
	 Stabilise();
 }

void GameOfLife_Cell_Brain::Build() {
	AddGate("and_0", "and", {"input_0", "input_1", "input_2"}, false);
	AddGate("and_1", "and", {"input_0", "input_1", "input_2"}, false);
	
	AddGate("and_3", "and", {"input_0", "input_1"}, false);
	AddGate("and_3", "and", {"input_0", "input_1"}, false);
	AddGate("and_3", "and", {"input_0", "input_1"}, false);
	
	AddGate("or_0", "or", {"input_0", "input_1"}, false);
	AddGate("or_1", "or", {"input_0", "input_1"}, false);
	AddGate("or_2", "or", {"input_0", "input_1"}, false);
	AddGate("or_3", "or", {"input_0", "input_1"}, false);
	
	AddGate("not_0", "not", {"input"}, false);
	AddGate("not_1", "not", {"input"}, false);
	AddGate("not_2", "not", {"input"}, false);
	AddGate("not_3", "not", {"input"}, false);
	
	Connect("count_0", "not_0", "input");
	Connect("count_0", "and_1", "input_0");
	
	Connect("count_1", "and_0", "input_1");
	Connect("count_1", "and_1", "input_1");
	
	Connect("count_2", "not_0", "input");
	
	ChildConnect("not_0", {"output", "and_0", "input_2"});
	ChildConnect("not_0", {"output", "and_1", "input_2"});
	
	ChildConnect("and_0", {"output", "or_0", "input_0"});
	ChildConnect("and_1", {"output", "or_0", "input_1"});
}
