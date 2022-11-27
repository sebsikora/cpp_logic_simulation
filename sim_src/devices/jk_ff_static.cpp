#include "c_structs.hpp"
#include "c_gates.hpp"
#include "c_device.hpp"
#include "jk_ff_static.h"

JK_FF_static::JK_FF_static(Device* parent_device_pointer, std::string name, bool monitor_on, std::vector<StateDescriptor> input_default_states) 
	: Device(parent_device_pointer, name, "jk_ff", {"j", "k", "clk"}, {"q", "not_q"}, monitor_on, input_default_states),
	m_nand1{this, "nand_1", {"input_0", "input_1", "input_2"}, false},
	m_nand2{this, "nand_2", {"input_0", "input_1", "input_2"}, false},
	m_nand3{this, "nand_3", {"input_0", "input_1"}, false},
	m_nand4{this, "nand_4", {"input_0", "input_1"}, false},
	m_nand5{this, "nand_5", {"input_0", "input_1"}, false},
	m_nand6{this, "nand_6", {"input_0", "input_1"}, false},
	m_nand7{this, "nand_7", {"input_0", "input_1"}, false},
	m_nand8{this, "nand_8", {"input_0", "input_1"}, false},
	m_not1{this, "not_1", false}
{
	// Following base class constructor (Device), we call the below overridden Build() method to populate the
	// specific device, then we call the base Stabilise() method to configure initial internal device component state.
	Build();
	Stabilise();
}

void JK_FF_static::Build() {
	// Add components.
	AddComponent(&m_nand1);
	AddComponent(&m_nand2);
	AddComponent(&m_nand3);
	AddComponent(&m_nand4);
	AddComponent(&m_nand5);
	AddComponent(&m_nand6);
	AddComponent(&m_nand7);
	AddComponent(&m_nand8);
	AddComponent(&m_not1);
	
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
}

Four_Bit_Counter_static::Four_Bit_Counter_static(Device* parent_device_pointer, std::string name, bool monitor_on, std::vector<StateDescriptor> input_default_states) 
	: Device(parent_device_pointer, name, "4_bit_counter", {"run", "clk"}, {"q_0", "q_1", "q_2", "q_3"}, monitor_on, input_default_states),
	m_jkff0{this, "jk_ff_0", false},
	m_jkff1{this, "jk_ff_1", false},
	m_jkff2{this, "jk_ff_2", false},
	m_jkff3{this, "jk_ff_3", false},
	m_and0{this, "and_0", {"input_0", "input_1"}, false},
	m_and1{this, "and_1", {"input_0", "input_1"}, false},
	m_and2{this, "and_2", {"input_0", "input_1"}, false}
{
	// Following base class constructor (Device), we call the below overridden Build() method to populate the
	// specific device, then we call the base Stabilise() method to configure initial internal device component state.
	Build();
	Stabilise();
}

void Four_Bit_Counter_static::Build() {
	// Add components.
	AddComponent(&m_jkff0);
	AddComponent(&m_jkff1);
	AddComponent(&m_jkff2);
	AddComponent(&m_jkff3);
	AddComponent(&m_and0);
	AddComponent(&m_and1);
	AddComponent(&m_and2);

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
}
