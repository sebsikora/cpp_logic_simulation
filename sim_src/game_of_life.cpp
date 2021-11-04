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
#include "devices.h"
#include "game_of_life.h"

GameOfLife_Cell_Decider::GameOfLife_Cell_Decider(Device* parent_device_pointer, std::string cell_name, bool monitor_on, std::vector<state_descriptor> in_pin_default_states) 
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

GameOfLife_Cell_SiblingSelector::GameOfLife_Cell_SiblingSelector(Device* parent_device_pointer, std::string cell_name, bool monitor_on, std::vector<state_descriptor> in_pin_default_states) 
 : Device(parent_device_pointer, cell_name, "g_o_l_cell_siblingselector", {"clk", "not_clear", "sibling_0", "sibling_1", "sibling_2", "sibling_3", "sibling_4", "sibling_5", "sibling_6", "sibling_7"}, {"selected_sibling", "update_flag"}, monitor_on, in_pin_default_states) {
	 // Following base class constructor (Device), we call the below overridden Build() method to populate the
	 // specific device, then we call the base Stabilise() method to configure initial internal device component state.
	 Build();
	 Stabilise();
 }

void GameOfLife_Cell_SiblingSelector::Build() {
	AddComponent(new N_Bit_Counter_C_ASC(this, "sibling_counter", 4, false, {{"not_clear", true}}));
	AddComponent(new N_Bit_Decoder(this, "sibling_counter_decoder", 3, false));
	AddComponent(new NxOne_Bit_Mux(this, "sibling_mux", 8, false));
	ChildConnect("sibling_counter", {"q_3", "parent", "update_flag"});
	ChildConnect("sibling_counter", {"q_3", "sibling_counter", "clear"});
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
	ChildConnect("sibling_counter_decoder", {"out_0", "sibling_mux", "sel_in_0"});
	ChildConnect("sibling_counter_decoder", {"out_1", "sibling_mux", "sel_in_1"});
	ChildConnect("sibling_counter_decoder", {"out_2", "sibling_mux", "sel_in_2"});
	ChildConnect("sibling_counter_decoder", {"out_3", "sibling_mux", "sel_in_3"});
	ChildConnect("sibling_counter_decoder", {"out_4", "sibling_mux", "sel_in_4"});
	ChildConnect("sibling_counter_decoder", {"out_5", "sibling_mux", "sel_in_5"});
	ChildConnect("sibling_counter_decoder", {"out_6", "sibling_mux", "sel_in_6"});
	ChildConnect("sibling_counter_decoder", {"out_7", "sibling_mux", "sel_in_7"});
	ChildConnect("sibling_mux", {"d_out", "parent", "selected_sibling"});
	MakeProbable();
}

GameOfLife_Cell::GameOfLife_Cell(Device* parent_device_pointer, std::string cell_name, bool monitor_on, std::vector<state_descriptor> in_pin_default_states) 
 : Device(parent_device_pointer, cell_name, "g_o_l_cell", {"clk", "not_clear_cycle", "not_clear_state", "not_preset_state", "sibling_0_in",
	 "sibling_1_in", "sibling_2_in", "sibling_3_in", "sibling_4_in", "sibling_5_in", "sibling_6_in", "sibling_7_in"},
	 {"sibling_0_out", "sibling_1_out", "sibling_2_out", "sibling_3_out", "sibling_4_out", "sibling_5_out", "sibling_6_out", "sibling_7_out"}, monitor_on, in_pin_default_states) {
	 // Following base class constructor (Device), we call the below overridden Build() method to populate the
	 // specific device, then we call the base Stabilise() method to configure initial internal device component state.
	 Build();
	 Stabilise();
 }

void GameOfLife_Cell::Build() {
	// Instantiate components.
	AddComponent(new GameOfLife_Cell_SiblingSelector(this, "selector", false, {{"not_clear", true}}));
	AddGate("not_0", "not");
	AddGate("and_0", "and", {"input_0", "input_1"});
	
	AddComponent(new N_Bit_Counter_C_ASC(this, "counter", 4, false, {{"not_clear", true}}));
	AddComponent(new GameOfLife_Cell_Decider(this, "decider", false));
	
	AddComponent(new JK_FF_ASPC(this, "state_ff", false, {{"not_c", true}, {"not_p", true}}));
	AddGate("and_1", "and", {"input_0", "input_1"});
	AddGate("not_1", "not");
	AddGate("and_2", "and", {"input_0", "input_1"});
	
	// Parent connections.
	Connect("clk", "selector", "clk");
	Connect("clk", "counter", "clk");
	Connect("clk", "state_ff", "clk");
	Connect("not_clear_cycle", "selector", "not_clear");
	Connect("not_clear_cycle", "counter", "not_clear");
	Connect("not_clear_state", "state_ff", "not_c");
	Connect("not_preset_state", "state_ff", "not_p");
	
	// "selector" connections.
	for (int i = 0; i < 8; i ++) {
		std::string cell_sibling_input_name = "sibling_" + std::to_string(i) + "_in";
		std::string selector_input_name = "sibling_" + std::to_string(i);
		Connect(cell_sibling_input_name, "selector", selector_input_name);
	}
	ChildConnect("selector", {"selected_sibling", "and_0", "input_0"});
	ChildConnect("selector", {"update_flag", "not_0"});
	ChildConnect("selector", {"update_flag", "counter", "clear"});
	ChildConnect("not_0", {"and_0", "input_1"});
	ChildConnect("and_0", {"counter", "run"});
	
	ChildConnect("counter", {"q_0", "decider", "count_in_0"});
	ChildConnect("counter", {"q_1", "decider", "count_in_1"});
	ChildConnect("counter", {"q_2", "decider", "count_in_2"});
	ChildMarkOutputNotConnected("counter", "q_3");
	
	ChildConnect("selector", {"update_flag", "and_1", "input_1"});
	ChildConnect("selector", {"update_flag", "and_2", "input_1"});
	ChildConnect("decider", {"alive_out", "and_1", "input_0"});
	ChildConnect("decider", {"alive_out", "not_1"});
	ChildConnect("not_1", {"and_2", "input_0"});
	
	ChildConnect("and_1", {"state_ff", "j"});
	ChildConnect("and_2", {"state_ff", "k"});
	
	ChildConnect("state_ff", {"q", "decider", "alive_in"});
	ChildConnect("state_ff", {"q", "parent", "sibling_0_out"});
	ChildConnect("state_ff", {"q", "parent", "sibling_1_out"});
	ChildConnect("state_ff", {"q", "parent", "sibling_2_out"});
	ChildConnect("state_ff", {"q", "parent", "sibling_3_out"});
	ChildConnect("state_ff", {"q", "parent", "sibling_4_out"});
	ChildConnect("state_ff", {"q", "parent", "sibling_5_out"});
	ChildConnect("state_ff", {"q", "parent", "sibling_6_out"});
	ChildConnect("state_ff", {"q", "parent", "sibling_7_out"});
	
	ChildMarkOutputNotConnected("state_ff", "not_q");
	ChildMakeProbable("counter");
	MakeProbable();
}

GameOfLife::GameOfLife(Device* parent_device_pointer, std::string name, int x_dimension, bool monitor_on, std::vector<state_descriptor> in_pin_default_states) 
 : Device(parent_device_pointer, name, "game_of_life", {"clk", "not_clear_cycle"}, {}, monitor_on, in_pin_default_states) {
	 // Following base class constructor (Device), we call the below overridden Build() method to populate the
	 // specific device, then we call the base Stabilise() method to configure initial internal device component state.
	 m_x_dimension = x_dimension;
	 m_cell_count = x_dimension * x_dimension;
	 ConfigureBusses(in_pin_default_states);
	 Build();
	 Stabilise();
 }

void GameOfLife::ConfigureBusses(std::vector<state_descriptor> in_pin_default_states) {
	std::vector<std::string> inputs_to_create;
	for (int cell_index = 0; cell_index < m_cell_count; cell_index ++) {
		std::string not_clear_identifier = "cell_" + std::to_string(cell_index) + "_not_clear_state";
		std::string not_preset_identifier = "cell_" + std::to_string(cell_index) + "_not_preset_state";
		inputs_to_create.push_back(not_clear_identifier);
		inputs_to_create.push_back(not_preset_identifier);
	}
	std::vector<std::string> outputs_to_create;
	for (int cell_index = 0; cell_index < m_cell_count; cell_index ++) {
		std::string output_identifier = "cell_" + std::to_string(cell_index) + "_state";
		outputs_to_create.push_back(output_identifier);
	}
	CreateInPins(inputs_to_create, in_pin_default_states);
	CreateOutPins(outputs_to_create);
}

void GameOfLife::Build() {
	// Instantiate cells and make main connections from game of life inputs and to outputs.
	int current_cell_id = 0;
	for (int row_index = 0; row_index < m_x_dimension; row_index ++) {
		for (int column_index = 0; column_index < m_x_dimension; column_index ++) {
			std::string cell_identifier = "cell_" + std::to_string(current_cell_id);
			AddComponent(new GameOfLife_Cell(this, cell_identifier, false, {{"not_clear_cycle", true}, {"not_clear_state", true}, {"not_preset_cycle", true}}));
			Connect("clk", cell_identifier, "clk");
			Connect(cell_identifier + "_not_clear_state", cell_identifier, "not_clear_state");
			Connect(cell_identifier + "_not_preset_state", cell_identifier, "not_preset_state");
			Connect("not_clear_cycle", cell_identifier, "not_clear_cycle");
			ChildConnect(cell_identifier, {"sibling_0_out", "parent", cell_identifier + "_state"});
			current_cell_id ++;
		}
	}
	current_cell_id = 0;
	for (int row_index = 0; row_index < m_x_dimension; row_index ++) {
		for (int column_index = 0; column_index < m_x_dimension; column_index ++) {
			std::string this_cell_identifier = "cell_" + std::to_string(current_cell_id);
			// Offsets to all sibling cells for cells away from the periphery.
			int target_cell_0 = current_cell_id - m_x_dimension;
			int target_cell_1 = current_cell_id - m_x_dimension + 1;
			int target_cell_2 = current_cell_id + 1;
			int target_cell_3 = current_cell_id + m_x_dimension + 1;
			int target_cell_4 = current_cell_id + m_x_dimension;
			int target_cell_5 = current_cell_id + m_x_dimension - 1;
			int target_cell_6 = current_cell_id - 1;
			int target_cell_7 = current_cell_id - m_x_dimension - 1;
			// Some of these change for cells on the periphery.
			if ((column_index == 0) && ((row_index != 0) && (row_index != (m_x_dimension - 1)))) {
				// Mid row, first column.
				target_cell_5 = current_cell_id + (2 * m_x_dimension) - 1;
				target_cell_6 = current_cell_id + m_x_dimension - 1;
				target_cell_7 = current_cell_id - 1;
			} else if ((column_index == (m_x_dimension - 1)) && ((row_index != 0) && (row_index != (m_x_dimension - 1)))) {
				// Mid row, last column.
				target_cell_1 = current_cell_id - (2 * m_x_dimension) + 1;
				target_cell_2 = current_cell_id - m_x_dimension + 1;
				target_cell_3 = current_cell_id + 1;
			} else if ((row_index == 0) && ((column_index != 0) && (column_index != (m_x_dimension - 1)))) {
				// Mid column, first row.
				target_cell_0 = current_cell_id + (m_x_dimension * (m_x_dimension - 1));
				target_cell_1 = current_cell_id + (m_x_dimension * (m_x_dimension - 1)) + 1;
				target_cell_7 = current_cell_id + (m_x_dimension * (m_x_dimension - 1)) - 1;
			} else if ((row_index == (m_x_dimension - 1)) && ((column_index != 0) && (column_index != (m_x_dimension - 1)))) {
				// Mid column, last row.
				target_cell_3 = current_cell_id - (m_x_dimension * (m_x_dimension - 1)) + 1;
				target_cell_4 = current_cell_id - (m_x_dimension * (m_x_dimension - 1));
				target_cell_5 = current_cell_id - (m_x_dimension * (m_x_dimension - 1)) - 1;
			} else if ((row_index == 0) && (column_index == 0)) {
				// Top-left corner.
				target_cell_0 = current_cell_id + (m_x_dimension * (m_x_dimension - 1));
				target_cell_1 = current_cell_id + (m_x_dimension * (m_x_dimension - 1)) + 1;
				target_cell_5 = current_cell_id + (2 * m_x_dimension) - 1;
				target_cell_6 = current_cell_id + m_x_dimension - 1;
				target_cell_7 = current_cell_id + (m_x_dimension * m_x_dimension) - 1;
			} else if ((row_index == 0) && (column_index == (m_x_dimension - 1))) {
				// Top-right corner.
				target_cell_0 = current_cell_id + (m_x_dimension * (m_x_dimension - 1));
				target_cell_1 = current_cell_id + (m_x_dimension * (m_x_dimension - 2)) + 1;
				target_cell_2 = current_cell_id - m_x_dimension + 1;
				target_cell_3 = current_cell_id + 1;
				target_cell_7 = current_cell_id + (m_x_dimension * (m_x_dimension - 1)) - 1;
			} else if ((row_index == (m_x_dimension - 1)) && (column_index == 0)) {
				// Bottom-left corner.
				target_cell_3 = current_cell_id - (m_x_dimension * (m_x_dimension - 1)) + 1;
				target_cell_4 = current_cell_id - (m_x_dimension * (m_x_dimension - 1));
				target_cell_5 = current_cell_id - (m_x_dimension * (m_x_dimension - 2)) - 1;
				target_cell_6 = current_cell_id + m_x_dimension - 1;
				target_cell_7 = current_cell_id - 1;
			} else if ((row_index == (m_x_dimension - 1)) && (column_index == (m_x_dimension - 1))) {
				// Bottom-right corner.
				target_cell_1 = current_cell_id - (2 * m_x_dimension) + 1;
				target_cell_2 = current_cell_id - m_x_dimension + 1;
				target_cell_3 = current_cell_id - (m_x_dimension * m_x_dimension) + 1;
				target_cell_4 = current_cell_id - (m_x_dimension * (m_x_dimension - 1));
				target_cell_5 = current_cell_id - (m_x_dimension * (m_x_dimension - 1)) - 1;
			}
			std::string target_cell_identifier_0 = "cell_" + std::to_string(target_cell_0);
			std::string target_cell_identifier_1 = "cell_" + std::to_string(target_cell_1);
			std::string target_cell_identifier_2 = "cell_" + std::to_string(target_cell_2);
			std::string target_cell_identifier_3 = "cell_" + std::to_string(target_cell_3);
			std::string target_cell_identifier_4 = "cell_" + std::to_string(target_cell_4);
			std::string target_cell_identifier_5 = "cell_" + std::to_string(target_cell_5);
			std::string target_cell_identifier_6 = "cell_" + std::to_string(target_cell_6);
			std::string target_cell_identifier_7 = "cell_" + std::to_string(target_cell_7);
			
			// Sibling 0.
			ChildConnect(this_cell_identifier, {"sibling_0_out", target_cell_identifier_0, "sibling_4_in"});
			// Sibling 1.
			ChildConnect(this_cell_identifier, {"sibling_1_out", target_cell_identifier_1, "sibling_5_in"});
			// Sibling 2.
			ChildConnect(this_cell_identifier, {"sibling_2_out", target_cell_identifier_2, "sibling_6_in"});
			// Sibling 3.
			ChildConnect(this_cell_identifier, {"sibling_3_out", target_cell_identifier_3, "sibling_7_in"});
			// Sibling 4.
			ChildConnect(this_cell_identifier, {"sibling_4_out", target_cell_identifier_4, "sibling_0_in"});
			// Sibling 5.
			ChildConnect(this_cell_identifier, {"sibling_5_out", target_cell_identifier_5, "sibling_1_in"});
			// Sibling 6.
			ChildConnect(this_cell_identifier, {"sibling_6_out", target_cell_identifier_6, "sibling_2_in"});
			// Sibling 7.
			ChildConnect(this_cell_identifier, {"sibling_7_out", target_cell_identifier_7, "sibling_3_in"});
			
			current_cell_id ++;
		}
	}
}
