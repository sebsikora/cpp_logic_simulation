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
#include <cmath>					// pow()

#include "c_core.h"					// Core simulator functionality
#include "devices.h"

JK_FF::JK_FF(Device* parent_device_pointer, std::string name, bool monitor_on, std::vector<state_descriptor> input_default_states) 
 : Device(parent_device_pointer, name, "jk_ff", {"j", "k", "clk"}, {"q", "not_q"}, monitor_on, input_default_states) {
	 // Following base class constructor (Device), we call the below overridden Build() method to populate the
	 // specific device, then we call the base Stabilise() method to configure initial internal device component state.
	 Build();
	 Stabilise();
 }

void JK_FF::Build() {
	// Instantiate components.
	// Can do it by directly instantiating the object.
	AddComponent(new Gate(this, "nand_1", "nand", {"input_0", "input_1", "input_2"}, false));
	// Or via a custom method.
	AddGate("nand_2", "nand", {"input_0", "input_1", "input_2"}, false);
	AddGate("nand_3", "nand", {"input_0", "input_1"}, false);
	AddGate("nand_4", "nand", {"input_0", "input_1"}, false);
	AddGate("nand_5", "nand", {"input_0", "input_1"}, false);
	AddGate("nand_6", "nand", {"input_0", "input_1"}, false);
	AddGate("nand_7", "nand", {"input_0", "input_1"}, false);
	AddGate("nand_8", "nand", {"input_0", "input_1"}, false);
	AddGate("not_1", "not");	// in_pins vector defaults to {} which is fine for not Gates, monitor_on defaults to false.
	
	// Interconnect components.
	ChildConnect("not_1", {"nand_5", "input_1"});
	ChildConnect("not_1", {"nand_6", "input_1"});
	
	ChildConnect("nand_1", {"nand_3", "input_0"});
	ChildConnect("nand_2", {"nand_4", "input_0"});
	ChildConnect("nand_3", {"nand_4", "input_1"});
	ChildConnect("nand_4", {"nand_3", "input_1"});
	
	ChildConnect("nand_3", {"nand_5", "input_0"});
	ChildConnect("nand_4", {"nand_6", "input_0"});
	
	ChildConnect("nand_5", {"nand_7", "input_0"});
	ChildConnect("nand_6", {"nand_8", "input_0"});
	ChildConnect("nand_7", {"nand_8", "input_1"});
	ChildConnect("nand_8", {"nand_7", "input_1"});
	
	ChildConnect("nand_7", {"nand_2", "input_1"});
	ChildConnect("nand_8", {"nand_1", "input_1"});
	
	ChildConnect("nand_7", {"parent", "q"});
	ChildConnect("nand_8", {"parent", "not_q"});
	
	// Connect device terminals to components.
	Connect("j", "nand_1", "input_0");
	Connect("k", "nand_2", "input_0");
	Connect("clk", "nand_1", "input_2");
	Connect("clk", "nand_2", "input_2");
	Connect("clk", "not_1");				// Not Gates created with only one in pin "input" so we can omit the target pin name parameter.
	
	// Add device to top-level probable list.
	//MakeProbable();
	//PrintInPinStates();
}

JK_FF_ASPC::JK_FF_ASPC(Device* parent_device_pointer, std::string name, bool monitor_on, std::vector<state_descriptor> input_default_states) 
 : Device(parent_device_pointer, name, "jk_ff_aspc", {"j", "k", "not_p", "not_c", "clk"}, {"q", "not_q"}, monitor_on, input_default_states) {
	 // Following base class constructor (Device), we call the below overridden Build() method to populate the
	 // specific device, then we call the base Stabilise() method to configure initial internal device component state.
	 Build();
	 Stabilise();
 }

void JK_FF_ASPC::Build() {
	// Instantiate components.
	// Can do it by directly instantiating the object.
	AddComponent(new Gate(this, "nand_1", "nand", {"input_0", "input_1", "input_2"}, false));
	// Or via a custom method.
	AddGate("nand_2", "nand", {"input_0", "input_1", "input_2"}, false);
	AddGate("nand_3", "nand", {"input_0", "input_1", "input_2"}, false);
	AddGate("nand_4", "nand", {"input_0", "input_1", "input_2"}, false);
	AddGate("nand_5", "nand", {"input_0", "input_1"}, false);
	AddGate("nand_6", "nand", {"input_0", "input_1"}, false);
	AddGate("nand_7", "nand", {"input_0", "input_1", "input_2"}, false);
	AddGate("nand_8", "nand", {"input_0", "input_1", "input_2"}, false);
	AddGate("not_1", "not", false);			// AddGate overloaded to allow optional inclusion of monitor_on option without specifying in pin names.
	
	// Interconnect components.
	ChildConnect("not_1", {"nand_5", "input_1"});
	ChildConnect("not_1", {"nand_6", "input_1"});
	
	ChildConnect("nand_1", {"nand_3", "input_0"});
	ChildConnect("nand_2", {"nand_4", "input_0"});
	ChildConnect("nand_3", {"nand_4", "input_1"});
	ChildConnect("nand_4", {"nand_3", "input_1"});
	
	ChildConnect("nand_3", {"nand_5", "input_0"});
	ChildConnect("nand_4", {"nand_6", "input_0"});
	
	ChildConnect("nand_5", {"nand_7", "input_0"});
	ChildConnect("nand_6", {"nand_8", "input_0"});
	ChildConnect("nand_7", {"nand_8", "input_1"});
	ChildConnect("nand_8", {"nand_7", "input_1"});
	
	ChildConnect("nand_7", {"nand_2", "input_1"});
	ChildConnect("nand_8", {"nand_1", "input_1"});
	
	ChildConnect("nand_7", {"parent", "q"});
	ChildConnect("nand_8", {"parent", "not_q"});
	
	// Connect device terminals to components.
	Connect("j", "nand_1", "input_0");
	Connect("k", "nand_2", "input_0");
	Connect("not_p", "nand_3", "input_2");
	Connect("not_p", "nand_7", "input_2");
	Connect("not_c", "nand_4", "input_2");
	Connect("not_c", "nand_8", "input_2");
	Connect("clk", "nand_1", "input_2");
	Connect("clk", "nand_2", "input_2");
	Connect("clk", "not_1");
	
	// Add device to top-level probable list.
	//~MakeProbable();
}

Four_Bit_Counter::Four_Bit_Counter(Device* parent_device_pointer, std::string name, bool monitor_on, std::vector<state_descriptor> input_default_states) 
 : Device(parent_device_pointer, name, "4_bit_counter", {"run", "clk"}, {"q_0", "q_1", "q_2", "q_3"}, monitor_on, input_default_states) {
	 // Following base class constructor (Device), we call the below overridden Build() method to populate the
	 // specific device, then we call the base Stabilise() method to configure initial internal device component state.
	 Build();
	 Stabilise();
 }

void Four_Bit_Counter::Build() {
	// Instantiate components.
	AddComponent(new JK_FF(this, "jk_ff_0", false));
	AddComponent(new JK_FF(this, "jk_ff_1", false));
	AddComponent(new JK_FF(this, "jk_ff_2", false));
	AddComponent(new JK_FF(this, "jk_ff_3", false));
	AddGate("and_0", "and", {"input_0", "input_1"}, false);
	AddGate("and_1", "and", {"input_0", "input_1"}, false);
	AddGate("and_2", "and", {"input_0", "input_1"}, false);
	// As an example, we can make individual gates probable, but only via the parent devices' method...
	//~ChildMakeProbable("and_0");
	//~ChildMakeProbable("jk_ff_0");
	
	// Connect device terminals to components.
	Connect("run", "jk_ff_0", "j");
	Connect("run", "jk_ff_0", "k");
	Connect("run", "and_0", "input_0");
	Connect("clk", "jk_ff_0", "clk");
	Connect("clk", "jk_ff_1", "clk");
	Connect("clk", "jk_ff_2", "clk");
	Connect("clk", "jk_ff_3", "clk");
	
	// Interconnect components.
	// Bit 0 just has the flip-flop connections
	ChildConnect("jk_ff_0", {"q", "parent", "q_0"});
	ChildConnect("jk_ff_0", {"q", "and_0", "input_1"});
	
	// Bit 1.
	ChildConnect("and_0", {"jk_ff_1", "j"});
	ChildConnect("and_0", {"jk_ff_1", "k"});
	ChildConnect("and_0", {"and_1", "input_0"});
	ChildConnect("jk_ff_1", {"q", "parent", "q_1"});
	ChildConnect("jk_ff_1", {"q", "and_1", "input_1"});
	
	// Bit 2.
	ChildConnect("and_1", {"jk_ff_2", "j"});
	ChildConnect("and_1", {"jk_ff_2", "k"});
	ChildConnect("and_1", {"and_2", "input_0"});
	ChildConnect("jk_ff_2", {"q", "parent", "q_2"});
	ChildConnect("jk_ff_2", {"q", "and_2", "input_1"});
	
	// Bit 3.
	ChildConnect("and_2", {"jk_ff_3", "j"});
	ChildConnect("and_2", {"jk_ff_3", "k"});
	ChildConnect("jk_ff_3", {"q", "parent", "q_3"});
	
	// Mark unused outputs as not connected.
	ChildMarkOutputNotConnected("jk_ff_0", "not_q");
	ChildMarkOutputNotConnected("jk_ff_1", "not_q");
	ChildMarkOutputNotConnected("jk_ff_2", "not_q");
	ChildMarkOutputNotConnected("jk_ff_3", "not_q");
	
	// Add device to top-level probable list.
	//~MakeProbable();
	//PrintInPinStates();
	//PrintOutPinStates();
}

N_Bit_Counter::N_Bit_Counter(Device* parent_device_pointer, std::string name, int width, bool monitor_on, std::vector<state_descriptor> input_default_states) 
 : Device(parent_device_pointer, name, "n_bit_counter", {"run", "clk"}, {}, monitor_on, input_default_states) {
	 // Following base class constructor (Device), we call the below overridden Build() method to populate the
	 // specific device, then we call the base Stabilise() method to configure initial internal device component state.
	if (width < 2) {
		width = 2;
	}
	m_width = width;
	ConfigureOutputs();
	Build();
	Stabilise();
 }

void N_Bit_Counter::ConfigureOutputs() {
	std::vector<std::string> outputs_to_create;
	for (int index = 0; index < m_width; index ++) {
		std::string output_identifier = "q_" + std::to_string(index);
		outputs_to_create.push_back(output_identifier);
	}
	CreateOutPins(outputs_to_create);
}

void N_Bit_Counter::Build() {
	// Instantiate and connect components for bit 0.
	AddComponent(new JK_FF(this, "jk_ff_0", false));
	Connect("run", "jk_ff_0", "j");
	Connect("run", "jk_ff_0", "k");
	Connect("clk", "jk_ff_0", "clk");
	ChildConnect("jk_ff_0", {"q", "parent", "q_0"});

	// Instantiate and connect components for bits > 0.
	for (int slice_index = 1; slice_index < m_width; slice_index ++) {
		std::string last_ff_identifier = "jk_ff_" + std::to_string(slice_index - 1);
		std::string last_and_identifier = "and_" + std::to_string(slice_index - 1);
		std::string new_and_identifier = "and_" + std::to_string(slice_index);
		AddGate(new_and_identifier, "and", {"input_0", "input_1"}, false);
		if (slice_index == 1) {
			Connect("run", new_and_identifier, "input_0");
		} else {
			ChildConnect(last_and_identifier, {new_and_identifier, "input_0"});
		}
		ChildConnect(last_ff_identifier, {"q", new_and_identifier, "input_1"});
		std::string new_ff_identifier = "jk_ff_" + std::to_string(slice_index);
		AddComponent(new JK_FF(this, new_ff_identifier, false));
		Connect("clk", new_ff_identifier, "clk");
		ChildConnect(new_and_identifier, {new_ff_identifier, "j"});
		ChildConnect(new_and_identifier, {new_ff_identifier, "k"});
		std::string parent_output_identifier = "q_" + std::to_string(slice_index);
		ChildConnect(new_ff_identifier, {"q", "parent", parent_output_identifier});
	}
	// Mark unused flip-flop outputs as not connected.
	for (int i = 0; i < m_width; i ++) {
		std::string ff_identifier = "jk_ff_" + std::to_string(i);
		ChildMarkOutputNotConnected(ff_identifier, "not_q");
	}
	//~MakeProbable();
}

N_Bit_Counter_ASC::N_Bit_Counter_ASC(Device* parent_device_pointer, std::string name, int width, bool monitor_on, std::vector<state_descriptor> input_default_states) 
 : Device(parent_device_pointer, name, "n_bit_counter", {"run", "clk", "not_clear"}, {}, monitor_on, input_default_states) {
	 // Following base class constructor (Device), we call the below overridden Build() method to populate the
	 // specific device, then we call the base Stabilise() method to configure initial internal device component state.
	if (width < 2) {
		width = 2;
	}
	m_width = width;
	ConfigureOutputs();
	Build();
	Stabilise();
 }

void N_Bit_Counter_ASC::ConfigureOutputs() {
	std::vector<std::string> outputs_to_create;
	for (int index = 0; index < m_width; index ++) {
		std::string output_identifier = "q_" + std::to_string(index);
		outputs_to_create.push_back(output_identifier);
	}
	CreateOutPins(outputs_to_create);
}

void N_Bit_Counter_ASC::Build() {
	// Instantiate and connect components for bits 0 & 1.
	// NOTE - If we don't instantiate the flip-flops with the right states on the asynchronous preset and clear
	// pins, they won't settle appropriately.
	
	// Instantiate and connect components for bit 0.
	AddComponent(new JK_FF_ASPC(this, "jk_ff_0", false, {{"not_p", true}, {"not_c", true}}));
	Connect("run", "jk_ff_0", "j");
	Connect("run", "jk_ff_0", "k");
	Connect("clk", "jk_ff_0", "clk");
	Connect("not_clear", "jk_ff_0", "not_c");
	Connect("true", "jk_ff_0", "not_p");
	ChildConnect("jk_ff_0", {"q", "parent", "q_0"});

	// Instantiate and connect components for bits > 0.
	for (int slice_index = 1; slice_index < m_width; slice_index ++) {
		std::string last_ff_identifier = "jk_ff_" + std::to_string(slice_index - 1);
		std::string last_and_identifier = "and_" + std::to_string(slice_index - 1);
		std::string new_and_identifier = "and_" + std::to_string(slice_index);
		AddGate(new_and_identifier, "and", {"input_0", "input_1"}, false);
		if (slice_index == 1) {
			Connect("run", new_and_identifier, "input_0");
		} else {
			ChildConnect(last_and_identifier, {new_and_identifier, "input_0"});
		}
		ChildConnect(last_ff_identifier, {"q", new_and_identifier, "input_1"});
		std::string new_ff_identifier = "jk_ff_" + std::to_string(slice_index);
		AddComponent(new JK_FF_ASPC(this, new_ff_identifier, false, {{"not_p", true}, {"not_c", true}}));
		Connect("clk", new_ff_identifier, "clk");
		Connect("not_clear", new_ff_identifier, "not_c");
		Connect("true", new_ff_identifier, "not_p");
		ChildConnect(new_and_identifier, {new_ff_identifier, "j"});
		ChildConnect(new_and_identifier, {new_ff_identifier, "k"});
		std::string parent_output_identifier = "q_" + std::to_string(slice_index);
		ChildConnect(new_ff_identifier, {"q", "parent", parent_output_identifier});
	}
	// Mark unused flip-flop outputs as not connected.
	for (int i = 0; i < m_width; i ++) {
		std::string ff_identifier = "jk_ff_" + std::to_string(i);
		ChildMarkOutputNotConnected(ff_identifier, "not_q");
	}
	//~MakeProbable();
}

N_Bit_Counter_C_ASC::N_Bit_Counter_C_ASC(Device* parent_device_pointer, std::string name, int width, bool monitor_on, std::vector<state_descriptor> input_default_states) 
 : Device(parent_device_pointer, name, "n_bit_counter", {"run", "clk", "clear", "not_clear"}, {}, monitor_on, input_default_states) {
	 // Following base class constructor (Device), we call the below overridden Build() method to populate the
	 // specific device, then we call the base Stabilise() method to configure initial internal device component state.
	if (width < 2) {
		width = 2;
	}
	m_width = width;
	ConfigureOutputs();
	Build();
	Stabilise();
 }

void N_Bit_Counter_C_ASC::ConfigureOutputs() {
	std::vector<std::string> outputs_to_create;
	for (int index = 0; index < m_width; index ++) {
		std::string output_identifier = "q_" + std::to_string(index);
		outputs_to_create.push_back(output_identifier);
	}
	CreateOutPins(outputs_to_create);
}

void N_Bit_Counter_C_ASC::Build() {
	// NOTE - If we don't instantiate the flip-flops with the right states on the asynchronous preset and clear
	// pins, they won't settle appropriately.
	AddGate("n_clear", "not");
	Connect("clear", "n_clear");
	
	// Instantiate components for bit 0.
	AddComponent(new JK_FF_ASPC(this, "jk_ff_0", false, {{"not_p", true}, {"not_c", true}}));
	AddGate("and_0_0", "and", {"input_0", "input_1"});
	AddGate("and_0_1", "and", {"input_0", "input_1"});
	AddGate("or_0_0", "or", {"input_0", "input_1"});

	// Connections for bit 0.
	Connect("clk", "jk_ff_0", "clk");
	Connect("true", "jk_ff_0", "not_p");
	Connect("not_clear", "jk_ff_0", "not_c");
	Connect("run", "and_0_0", "input_0");
	Connect("run", "and_0_1", "input_0");
	ChildConnect("n_clear", {"and_0_0", "input_1"});
	ChildConnect("n_clear", {"and_0_1", "input_1"});
	Connect("clear", "or_0_0", "input_1");
	ChildConnect("and_0_0", {"jk_ff_0", "j"});
	ChildConnect("and_0_1", {"or_0_0", "input_0"});
	ChildConnect("or_0_0", {"jk_ff_0", "k"});
	ChildConnect("jk_ff_0", {"q", "parent", "q_0"});
	ChildMarkOutputNotConnected("jk_ff_0", "not_q");
	
	// Instantiate components for bit 1.
	AddComponent(new JK_FF_ASPC(this, "jk_ff_1", false, {{"not_p", true}, {"not_c", true}}));
	AddGate("and_1_0", "and", {"input_0", "input_1"});
	AddGate("and_1_1", "and", {"input_0", "input_1"});
	AddGate("and_1_2", "and", {"input_0", "input_1"});
	AddGate("or_1_0", "or", {"input_0", "input_1"});
	
	// Connections for bit 1.
	Connect("clk", "jk_ff_1", "clk");
	Connect("true", "jk_ff_1", "not_p");
	Connect("not_clear", "jk_ff_1", "not_c");
	Connect("run", "and_1_0", "input_1");
	ChildConnect("jk_ff_0", {"q", "and_1_0", "input_0"});
	ChildConnect("and_1_0", {"and_1_1", "input_0"});
	ChildConnect("and_1_0", {"and_1_2", "input_0"});
	ChildConnect("n_clear", {"and_1_1", "input_1"});
	ChildConnect("n_clear", {"and_1_2", "input_1"});
	ChildConnect("and_1_1", {"jk_ff_1", "j"});
	ChildConnect("and_1_2", {"or_1_0", "input_0"});
	Connect("clear", "or_1_0", "input_1");
	ChildConnect("or_1_0", {"jk_ff_1", "k"});
	ChildConnect("jk_ff_1", {"q", "parent", "q_1"});
	ChildMarkOutputNotConnected("jk_ff_1", "not_q");
	
	// Instantiate and connect components for bits > 1.
	for (int slice_index = 2; slice_index < m_width; slice_index ++) {
		std::string new_ff_identifier = "jk_ff_" + std::to_string(slice_index);
		AddComponent(new JK_FF_ASPC(this, new_ff_identifier, false, {{"not_p", true}, {"not_c", true}}));
		std::string new_and_identifier_0 = "and_" + std::to_string(slice_index) + "_0";
		std::string new_and_identifier_1 = "and_" + std::to_string(slice_index) + "_1";
		std::string new_and_identifier_2 = "and_" + std::to_string(slice_index) + "_2";
		AddGate(new_and_identifier_0, "and", {"input_0", "input_1"}, false);
		AddGate(new_and_identifier_1, "and", {"input_0", "input_1"}, false);
		AddGate(new_and_identifier_2, "and", {"input_0", "input_1"}, false);
		std::string new_or_identifier = "or_" + std::to_string(slice_index) + "_0";
		AddGate(new_or_identifier, "or", {"input_0", "input_1"}, false);
		
		Connect("clk", new_ff_identifier, "clk");
		Connect("true", new_ff_identifier, "not_p");
		Connect("not_clear", new_ff_identifier, "not_c");
		std::string last_and_identifier = "and_" + std::to_string(slice_index - 1) + "_0";
		std::string last_ff_identifier = "jk_ff_" + std::to_string(slice_index - 1);
		ChildConnect(last_and_identifier, {new_and_identifier_0, "input_1"});
		ChildConnect(last_ff_identifier, {"q", new_and_identifier_0, "input_0"});
		
		// Other connections for this additional bit do not vary.
		ChildConnect(new_and_identifier_0, {new_and_identifier_1, "input_0"});
		ChildConnect(new_and_identifier_0, {new_and_identifier_2, "input_0"});
		ChildConnect("n_clear", {new_and_identifier_1, "input_1"});
		ChildConnect("n_clear", {new_and_identifier_2, "input_1"});
		Connect("clear", new_or_identifier, "input_1");
		ChildConnect(new_and_identifier_1, {new_ff_identifier, "j"});
		ChildConnect(new_and_identifier_2, {new_or_identifier, "input_0"});
		ChildConnect(new_or_identifier, {new_ff_identifier, "k"});
		std::string output_identifier = "q_" + std::to_string(slice_index);
		ChildConnect(new_ff_identifier, {"q", "parent", output_identifier});
		ChildMarkOutputNotConnected(new_ff_identifier, "not_q");
	}
	//~MakeProbable();
}

One_Bit_Register::One_Bit_Register(Device* parent_device_pointer, std::string name, bool monitor_on, std::vector<state_descriptor> input_default_states) 
 : Device(parent_device_pointer, name, "1_bit_register", {"d_in", "load", "clr", "clk"}, {"d_out"}, monitor_on, input_default_states) {
	 // Following base class constructor (Device), we call the below overridden Build() method to populate the
	 // specific device, then we call the base Stabilise() method to configure initial internal device component state.
	 Build();
	 Stabilise();
 }

void One_Bit_Register::Build() {
	// Instantiate components.
	AddComponent(new JK_FF(this, "jk_ff_0", false));
	AddGate("and_0", "and", {"input_0", "input_1", "input_2"}, false);
	AddGate("and_1", "and", {"input_0", "input_1", "input_2"}, false);
	AddGate("and_2", "and", {"input_0", "input_1"}, false);
	AddGate("or_0", "or", {"input_0", "input_1"}, false);
	AddGate("not_0", "not", {}, false);
	AddGate("not_1", "not", {}, false);
	AddGate("not_2", "not", {}, false);
	AddGate("not_3", "not", {}, false);
	
	// Interconnect components.
	ChildConnect("and_0", {"jk_ff_0", "j"});
	ChildConnect("and_1", {"or_0", "input_0"});
	ChildConnect("and_2", {"or_0", "input_1"});
	ChildConnect("or_0", {"jk_ff_0", "k"});
	ChildConnect("not_0", {"and_0", "input_2"});
	ChildConnect("not_1", {"and_1", "input_0"});
	ChildConnect("not_2", {"and_1", "input_2"});
	ChildConnect("not_3", {"and_2", "input_0"});
	ChildConnect("jk_ff_0", {"q", "parent", "d_out"});
	
	// Connect device terminals to components.
	Connect("load", "and_0", "input_1");
	Connect("load", "and_1", "input_1");
	Connect("load", "not_3");
	Connect("d_in", "and_0", "input_0");
	Connect("d_in", "not_1");
	Connect("clr", "not_0");
	Connect("clr", "not_2");
	Connect("clr", "and_2", "input_1");
	Connect("clk", "jk_ff_0", "clk");
	
	// Mark unused flip-flop outputs as not connected.
	ChildMarkOutputNotConnected("jk_ff_0", "not_q");
	
	// Add device to top-level probable list.
	//~MakeProbable();
}

N_Bit_Register::N_Bit_Register(Device* parent_device_pointer, std::string name, int width, bool monitor_on, std::vector<state_descriptor> input_default_states) 
 : Device(parent_device_pointer, name, "n_bit_register", {"load", "clr", "clk"}, {}, monitor_on, input_default_states) {
	 m_bus_width = width;
	 ConfigureBusses(input_default_states);
	 Build();
	 Stabilise();
 }
 
void N_Bit_Register::ConfigureBusses(std::vector<state_descriptor> input_default_states) {
	std::vector<std::string> inputs_to_create;
	for (int index = 0; index < m_bus_width; index ++) {
		std::string input_identifier = "d_in_" + std::to_string(index);
		inputs_to_create.push_back(input_identifier);
	}
	std::vector<std::string> outputs_to_create;
	for (int index = 0; index < m_bus_width; index ++) {
		std::string output_identifier = "d_out_" + std::to_string(index);
		outputs_to_create.push_back(output_identifier);
	}
	CreateInPins(inputs_to_create, input_default_states);
	CreateOutPins(outputs_to_create);
}

void N_Bit_Register::Build() {
	// Instantiate components.
	for (int i = 0; i < m_bus_width; i ++) {
		std::string identifier = "reg_" + std::to_string(i);
		AddComponent(new One_Bit_Register(this, identifier, false));
	}
	
	// Interconnect components.
	for (int i = 0; i < m_bus_width; i ++) {
		std::string reg_identifier = "reg_" + std::to_string(i);
		std::string parent_pin_identifier = "d_out_" + std::to_string(i);
		ChildConnect(reg_identifier, {"d_out", "parent", parent_pin_identifier});
	}
	
	// Connect device terminals to components.
	for (int i = 0; i < m_bus_width; i ++) {
		std::string reg_identifier = "reg_" + std::to_string(i);
		std::string parent_pin_identifier = "d_in_" + std::to_string(i);
		Connect("clk", reg_identifier, "clk");
		Connect("load", reg_identifier, "load");
		Connect("clr", reg_identifier, "clr");
		Connect(parent_pin_identifier, reg_identifier, "d_in");
	}
	
	// Add device to top-level probable list.
	//~MakeProbable();
	//~PrintInPinStates();
}

NxOne_Bit_Mux::NxOne_Bit_Mux(Device* parent_device_pointer, std::string name, int input_count, bool monitor_on, std::vector<state_descriptor> input_default_states) 
 : Device(parent_device_pointer, name, "nx1_bit_mux", {}, {"d_out"}, monitor_on, input_default_states) {
	 m_input_count = input_count;
	 ConfigureBusses(input_default_states);
	 Build();
	 Stabilise();
	 //~PrintInPinStates();
	 //~PrintOutPinStates();
}

void NxOne_Bit_Mux::ConfigureBusses(std::vector<state_descriptor> input_default_states) {
	std::vector<std::string> inputs_to_create;
	// Input data bus.
	for (int index = 0; index < m_input_count; index ++) {
		std::string input_identifier = "d_in_" + std::to_string(index);
		inputs_to_create.push_back(input_identifier);
	}
	// Input selector bus.
	//m_sel_bus_width = (int)std::ceil(std::log(m_input_count)/std::log(2));
	for (int i = 0; i < m_input_count; i ++) {
		std::string input_identifier = "sel_in_" + std::to_string(i);
		inputs_to_create.push_back(input_identifier);
	}
	CreateInPins(inputs_to_create, input_default_states);
}

void NxOne_Bit_Mux::Build() {
	// N-input or gate.
	std::vector<std::string> or_gate_input_identifiers;
	for (int i = 0; i < m_input_count; i ++) {
		std::string input_identifier = "input_" + std::to_string(i);
		or_gate_input_identifiers.push_back(input_identifier);
	}
	AddGate("or_0", "or", or_gate_input_identifiers, false);
	ChildConnect("or_0", {"parent", "d_out"});

	// And gates.
	for (int i = 0; i < m_input_count; i ++) {
		std::string and_gate_identifier = "and_" + std::to_string(i);
		AddGate(and_gate_identifier, "and", {"input_0", "input_1"}, false);
		std::string parent_input_pin_identifier = "d_in_" + std::to_string(i);
		std::string parent_select_pin_identifier = "sel_in_" + std::to_string(i);
		Connect(parent_input_pin_identifier, and_gate_identifier, "input_0");
		Connect(parent_select_pin_identifier, and_gate_identifier, "input_1");
		std::string or_gate_input_identifier = "input_" + std::to_string(i);
		ChildConnect(and_gate_identifier, {"or_0", or_gate_input_identifier});
	}
	
	//~// Add device to top-level probable list.
	//~MakeProbable();
}

N_Bit_Decoder::N_Bit_Decoder(Device* parent_device_pointer, std::string name, int select_bus_width, bool monitor_on, std::vector<state_descriptor> input_default_states) 
 : Device(parent_device_pointer, name, "n_bit_decoder", {}, {}, monitor_on, input_default_states) {
	m_select_bus_width = select_bus_width;
	if (m_select_bus_width < 1) {
		m_select_bus_width = 1;
	}
	ConfigureBusses(input_default_states);
	Build();
	Stabilise();
	//~PrintInPinStates();
	//~PrintOutPinStates();
 }
 
void N_Bit_Decoder::ConfigureBusses(std::vector<state_descriptor> input_default_states) {
	std::vector<std::string> inputs_to_create;
	for (int index = 0; index < m_select_bus_width; index ++) {
		std::string input_identifier = "sel_" + std::to_string(index);
		inputs_to_create.push_back(input_identifier);
	}
	m_output_bus_width = pow(2, m_select_bus_width);
	std::vector<std::string> outputs_to_create;
	for (int index = 0; index < m_output_bus_width; index ++) {
		std::string output_identifier = "out_" + std::to_string(index);
		outputs_to_create.push_back(output_identifier);
	}
	CreateInPins(inputs_to_create, input_default_states);
	CreateOutPins(outputs_to_create);
}

void N_Bit_Decoder::Build() {
	// The N select inputs each need a complement.
	for (int i = 0; i < m_select_bus_width; i ++) {
		std::string not_gate_identifier = "not_" + std::to_string(i);
		AddGate(not_gate_identifier, "not", {}, false);
		std::string sel_input_identifier = "sel_" + std::to_string(i);
		Connect(sel_input_identifier, not_gate_identifier);
	}
	// The 2**N outputs each need an N-input AND Gate.
	// First, generate input terminal names.
	std::vector<std::string> input_terminal_identifiers = {};
	for (int i = 0; i < m_select_bus_width; i ++) {
		std::string terminal_identifier = "input_" + std::to_string(i);
		input_terminal_identifiers.push_back(terminal_identifier);
	}
	// Next, add the N**2 x N-input AND Gates...
	for (int output_index = 0; output_index < m_output_bus_width; output_index ++) {
		std::string and_gate_identifier = "and_" + std::to_string(output_index);
		AddGate(and_gate_identifier, "and", input_terminal_identifiers, false);
		// ...then, determine which AND Gate inputs should be connected directly to the select bus inputs, and
		// which to their complements.
		//
		// We can determine the pattern according to the inverse of the bits of the output selection value as follows:
		//
		// If the value is 0 the selection bit pattern will be 000. The inverse of this is 111.
		// All three inputs are connected to the complementary selection input.
		// If the value is 6, the selection bit pattern will be 110. The inverse of this is 001.
		// The first two inputs are connected to the direct selection input, the third to the complement.
		for (int input_pin_index = 0; input_pin_index < m_select_bus_width; input_pin_index ++) {
			std::string input_pin_identifier = "input_" + std::to_string(input_pin_index);
			// Is this bit position of the encoded output number a '0'?
			if (!((output_index & (1 << input_pin_index)) == (1 << input_pin_index))) {
				// This input needs to be connected to the complementary input.
				std::string not_gate_identifier = "not_" + std::to_string(input_pin_index);
				ChildConnect(not_gate_identifier, {and_gate_identifier, input_terminal_identifiers[input_pin_index]});
			} else {
				// This input needs to be connected directly to the select input.
				std::string select_input_identifier = "sel_" + std::to_string(input_pin_index);
				Connect(select_input_identifier, and_gate_identifier, input_terminal_identifiers[input_pin_index]);
			}
		}
		std::string output_pin_identifier = "out_" + std::to_string(output_index);
		ChildConnect(and_gate_identifier, {"parent", output_pin_identifier});
	}
	//~MakeProbable();
	//~PrintInPinStates();
	//~PrintOutPinStates();
}

NxM_Bit_Mux::NxM_Bit_Mux(Device* parent_device_pointer, std::string name, int bus_count, int bus_width, bool monitor_on, std::vector<state_descriptor> input_default_states) 
 : Device(parent_device_pointer, name, "nxm_bit_mux", {}, {}, monitor_on, input_default_states) {
	 m_d_bus_width = bus_width;
	 m_d_bus_count = bus_count;
	 if (m_d_bus_width < 2) {
		 m_d_bus_width = 2;
	 }
	 if (m_d_bus_count < 2) {
		 m_d_bus_count = 2;
	 }
	 ConfigureBusses(input_default_states);
	 Build();
	 Stabilise();
 }
 
void NxM_Bit_Mux::ConfigureBusses(std::vector<state_descriptor> input_default_states) {
	std::vector<std::string> inputs_to_create;
	// Input data busses.
	for (int i = 0; i < m_d_bus_count; i ++) {
		for (int j = 0; j < m_d_bus_width; j ++) {
			std::string input_identifier = "d_in_" + std::to_string(i) + "_" + std::to_string(j);
			inputs_to_create.push_back(input_identifier);
		}
	}
	// Input selector bus.
	m_s_bus_width = (int)std::ceil(std::log(m_d_bus_count)/std::log(2));
	for (int i = 0; i < m_s_bus_width; i ++) {
		std::string input_identifier = "sel_" + std::to_string(i);
		inputs_to_create.push_back(input_identifier);
	}
	// Output data bus.
	std::vector<std::string> outputs_to_create;
	for (int i = 0; i < m_d_bus_width; i ++) {
		std::string output_identifier = "d_out_" + std::to_string(i);
		outputs_to_create.push_back(output_identifier);
	}
	CreateInPins(inputs_to_create, input_default_states);
	CreateOutPins(outputs_to_create);
}

void NxM_Bit_Mux::Build() {
	// First we will add an N_Bit_Decoder to decode the input selection.
	AddComponent(new N_Bit_Decoder(this, "selection_decoder", m_s_bus_width, true));
	
	// We will have created a wide enough select bus to address m_data_bus_count input data busses,	but
	// unless m_data_bus_count is a round power of 2, we will end up with some decoder output lines
	// left unused. We need to determine how many, and mark them as connected or the connection checker
	// will complain.
	int spare_decoder_outputs = pow(m_s_bus_width, 2) - m_d_bus_count;
	int first_unused_output_index = m_d_bus_count;
	for (int i = first_unused_output_index; i < (first_unused_output_index + spare_decoder_outputs); i ++) {
		std::string decoder_output_name = "out_" + std::to_string(i);
		ChildMarkOutputNotConnected("selection_decoder", decoder_output_name);
	}
	
	// Connect NxM_Bit_Mux selection inputs to decoder selection inputs.
	for (int i = 0; i < m_s_bus_width; i ++) {
		std::string selection_input_identifier = "sel_" + std::to_string(i);
		Connect(selection_input_identifier, "selection_decoder", selection_input_identifier);
	}
	// Add M Nx1-bit muxes.
	for (int i = 0; i < m_d_bus_width; i ++) {
		std::string mux_name = "mux_" + std::to_string(i);
		AddComponent(new NxOne_Bit_Mux(this, mux_name, m_d_bus_count, false));
	}
	// Each Nx1-bit mux has m_data_bus_count select inputs, each of which is connected to the corresponding decoder output.
	for (int i = 0; i < m_d_bus_width; i ++) {
		std::string mux_name = "mux_" + std::to_string(i);
		for (int j = 0; j < m_d_bus_count; j ++) {
			std::string decoder_output_identifier = "out_" + std::to_string(j);
			std::string mux_selector_identifier = "sel_in_" + std::to_string(j);
			ChildConnect("selection_decoder", {decoder_output_identifier, mux_name, mux_selector_identifier});
		}
	}
	// Each Nx1-bit input handles a data bus position, across multiple inputs.
	// IE - Nx1-bit mux 0 takes as input in_0_0 and in_1_0. Mux 1 takes as input in_0_1 and in_1_1.
	for (int i = 0; i < m_d_bus_width; i ++) {
		std::string mux_name = "mux_" + std::to_string(i);
		for (int j = 0; j < m_d_bus_count; j ++) {
			std::string parent_input_identifier = "d_in_" + std::to_string(j) + "_" + std::to_string(i);
			std::string mux_input_identifier = "d_in_" + std::to_string(j);
			//std::cout << parent_input_identifier << " " << mux_name << " " << mux_input_identifier << std::endl;
			Connect(parent_input_identifier, mux_name, mux_input_identifier);
		}
	}
	// Lastly, connect the Nx1-bit mux outputs to the parent outputs.
	for (int i = 0; i < m_d_bus_width; i ++) {
		std::string mux_name = "mux_" + std::to_string(i);
		std::string parent_output_identifier = "d_out_" + std::to_string(i);
		ChildConnect(mux_name, {"d_out", "parent", parent_output_identifier});
	}
	
	// Add device to top-level probable list.
	//~MakeProbable();
	//~PrintInPinStates();
	//~PrintOutPinStates();
}

