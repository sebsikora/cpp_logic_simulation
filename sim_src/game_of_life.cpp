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
#include "devices.h"
#include "game_of_life.h"

GameOfLife_Cell_Decider::GameOfLife_Cell_Decider(Device* parent_device_pointer, std::string cell_name, bool monitor_on, std::unordered_map<std::string, bool> in_pin_default_states) 
 : Device(parent_device_pointer, cell_name, "g_o_l_cell_decider", {"count_in_0", "count_in_1", "count_in_2", "alive_in"}, {"alive_out"}, monitor_on, in_pin_default_states) {
	 // Following base class constructor (Device), we call the below overridden Build() method to populate the
	 // specific device, then we call the base Stabilise() method to configure initial internal device component state.
	 Build();
	 Stabilise();
 }

void GameOfLife_Cell_Decider::Build() {
	AddGate("not_0", "not");
	AddGate("not_1", "not");
	
	AddGate("and_0", "and", {"input_0", "input_1", "input_2"}, false);
	AddGate("and_1", "and", {"input_0", "input_1", "input_2"}, false);
	AddGate("or_0", "or", {"input_0", "input_1"}, false);
	AddGate("not_2", "not");
	AddGate("and_2", "and", {"input_0", "input_1"}, false);
	AddGate("and_3", "and", {"input_0", "input_1"}, false);
	AddGate("or_1", "or", {"input_0", "input_1"}, false);
	
	Connect("count_in_0", "not_0");
	ChildConnect("not_0", {"and_0", "input_0"});
	Connect("count_in_0", "and_1", "input_0");
	
	Connect("count_in_1", "and_0", "input_1");
	Connect("count_in_1", "and_1", "input_1");
	
	Connect("count_in_2", "not_1");
	ChildConnect("not_1", {"and_0", "input_2"});
	ChildConnect("not_1", {"and_1", "input_2"});
	
	ChildConnect("and_0", {"or_0", "input_0"});
	ChildConnect("and_1", {"or_0", "input_1"});
	ChildConnect("or_0", {"and_2", "input_0"});
	Connect("alive_in", "and_2", "input_1");
	
	Connect("alive_in", "not_2");
	ChildConnect("and_1", {"and_3", "input_0"});
	ChildConnect("not_2", {"and_3", "input_1"});
	
	ChildConnect("and_2", {"or_1", "input_0"});
	ChildConnect("and_3", {"or_1", "input_1"});
	ChildConnect("or_1", {"parent", "alive_out"});
	MakeProbable();
}

GameOfLife_Cell_SiblingSelector::GameOfLife_Cell_SiblingSelector(Device* parent_device_pointer, std::string cell_name, bool monitor_on, std::unordered_map<std::string, bool> in_pin_default_states) 
 : Device(parent_device_pointer, cell_name, "g_o_l_cell_siblingselector", {"clk", "not_clear", "sibling_0", "sibling_1", "sibling_2", "sibling_3", "sibling_4", "sibling_5", "sibling_6", "sibling_7"}, {"selected_sibling"}, monitor_on, in_pin_default_states) {
	 // Following base class constructor (Device), we call the below overridden Build() method to populate the
	 // specific device, then we call the base Stabilise() method to configure initial internal device component state.
	 Build();
	 Stabilise();
 }

void GameOfLife_Cell_SiblingSelector::Build() {
	AddComponent(new N_Bit_Counter_ASC(this, "sibling_counter", 3, false, {{"not_clear", true}}));
	AddComponent(new N_Bit_Decoder(this, "sibling_counter_decoder", 3, false));
	AddComponent(new NxOne_Bit_Mux(this, "sibling_mux", 8, false));
	Connect("not_clear", "sibling_counter", "not_clear");
	Connect("true", "sibling_counter", "run");
	Connect("clk", "sibling_counter", "clk");
	for (int i = 0; i < 8; i ++) {
		std::string input_pin_name = "sibling_" + std::to_string(i);
		std::string sibling_mux_pin_name = "d_in_" + std::to_string(i);
		Connect(input_pin_name, "sibling_mux", sibling_mux_pin_name);
	}
	ChildConnect("sibling_counter", {"q_0", "sibling_counter_decoder", "sel_0"});
	ChildConnect("sibling_counter", {"q_1", "sibling_counter_decoder", "sel_1"});
	ChildConnect("sibling_counter", {"q_2", "sibling_counter_decoder", "sel_2"});
	ChildMakeProbable("sibling_counter");
	ChildConnect("sibling_counter_decoder", {"out_0", "sibling_mux", "sel_in_0"});
	ChildConnect("sibling_counter_decoder", {"out_1", "sibling_mux", "sel_in_1"});
	ChildConnect("sibling_counter_decoder", {"out_2", "sibling_mux", "sel_in_2"});
	ChildConnect("sibling_counter_decoder", {"out_3", "sibling_mux", "sel_in_3"});
	ChildConnect("sibling_counter_decoder", {"out_4", "sibling_mux", "sel_in_4"});
	ChildConnect("sibling_counter_decoder", {"out_5", "sibling_mux", "sel_in_5"});
	ChildConnect("sibling_counter_decoder", {"out_6", "sibling_mux", "sel_in_6"});
	ChildConnect("sibling_counter_decoder", {"out_7", "sibling_mux", "sel_in_7"});
	ChildMakeProbable("sibling_counter_decoder");
	ChildConnect("sibling_mux", {"d_out", "parent", "selected_sibling"});
	MakeProbable();
}

GameOfLife_Cell::GameOfLife_Cell(Device* parent_device_pointer, std::string cell_name, bool monitor_on, std::unordered_map<std::string, bool> in_pin_default_states) 
 : Device(parent_device_pointer, cell_name, "g_o_l_cell", {"clk", "not_clear_cycle", "not_clear_state", "not_preset_state", "sibling_0_in", "sibling_1_in", "sibling_2_in", "sibling_3_in", "sibling_4_in", "sibling_5_in", "sibling_6_in", "sibling_7_in"}, {"sibling_0_out", "sibling_1_out", "sibling_2_out", "sibling_3_out", "sibling_4_out", "sibling_5_out", "sibling_6_out", "sibling_7_out"}, monitor_on, in_pin_default_states) {
	 // Following base class constructor (Device), we call the below overridden Build() method to populate the
	 // specific device, then we call the base Stabilise() method to configure initial internal device component state.
	 Build();
	 Stabilise();
 }

void GameOfLife_Cell::Build() {
	// Instantiate components.
	AddComponent(new GameOfLife_Cell_Decider(this, "decider", false));
	AddComponent(new GameOfLife_Cell_SiblingSelector(this, "selector", false, {{"not_clear", true}}));
	AddComponent(new N_Bit_Counter_ASC(this, "counter", 3, false, {{"not_clear", true}}));
	AddGate("state_ff_not", "not");
	AddComponent(new JK_FF_ASPC(this, "state_ff", false, {{"not_c", true}, {"not_p", true}}));

	// "counter" connections.
	Connect("clk", "counter", "clk");
	Connect("not_clear_cycle", "counter", "not_clear");
	ChildConnect("counter", {"q_0", "decider", "count_in_0"});
	ChildConnect("counter", {"q_1", "decider", "count_in_1"});
	ChildConnect("counter", {"q_2", "decider", "count_in_2"});
	
	// "decider" connections.
	ChildConnect("decider", {"alive_out", "state_ff", "j"});
	ChildConnect("decider", {"alive_out", "state_ff_not"});
	ChildConnect("state_ff_not", {"state_ff", "k"});
	
	// "state_ff" connections.
	Connect("not_clear_state", "state_ff", "not_c");
	Connect("not_preset_state", "state_ff", "not_p");
	Connect("clk", "state_ff", "clk");
	ChildConnect("state_ff", {"q", "decider", "alive_in"});
	for (int i = 0; i < 8; i ++) {
		std::string cell_sibling_output_name = "sibling_" + std::to_string(i) + "_out";
		ChildConnect("state_ff", {"q", "parent", cell_sibling_output_name});
	}
	
	// "selector" connections.
	Connect("clk", "selector", "clk");
	Connect("not_clear_cycle", "selector", "not_clear");
	for (int i = 0; i < 8; i ++) {
		std::string cell_sibling_input_name = "sibling_" + std::to_string(i) + "_in";
		std::string selector_input_name = "sibling_" + std::to_string(i);
		Connect(cell_sibling_input_name, "selector", selector_input_name);
	}
	ChildConnect("selector", {"selected_sibling", "counter", "run"});
	
	MakeProbable();
}
