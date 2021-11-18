// quad_sr_latch.cpp

#include "c_core.h"	             // Core simulator functionality
#include "sr_latch.h"            // Previously defined SR latch device
#include "quad_sr_latch.h"       // Our new device

Quad_SR_Latch::Quad_SR_Latch(Device* parent_device_pointer, std::string name, bool monitor_on, std::vector<state_descriptor> input_default_states) 
 : Device(parent_device_pointer, name, "quad_sr_latch", {"S_0", "S_1", "S_2", "S_3", "R_0", "R_1", "R_2", "R_3", "R_All"}, {"Out_0", "Out_1", "Out_2", "Out_3"}, monitor_on, input_default_states) {
	// Following base class constructor (Device), we call the below overridden Build() method to populate the
	// specific device, then we call the base Stabilise() method to configure initial internal device component state.
	Build();
	Stabilise();
}

void Quad_SR_Latch::Build() {
	// Instantiate latches.
	//
	AddComponent(new SR_Latch(this, "sr_latch_0"));
	AddComponent(new SR_Latch(this, "sr_latch_1"));
	AddComponent(new SR_Latch(this, "sr_latch_2"));
	AddComponent(new SR_Latch(this, "sr_latch_3"));

	// Instantiate gates.
	//
	AddGate("or_0", "or", {"input_0", "input_1"});
	AddGate("or_1", "or", {"input_0", "input_1"});
	AddGate("or_2", "or", {"input_0", "input_1"});
	AddGate("or_3", "or", {"input_0", "input_1"});

	Connect("S_0", "sr_latch_0", "S");
	Connect("S_1", "sr_latch_1", "S");
	Connect("S_2", "sr_latch_2", "S");
	Connect("S_3", "sr_latch_3", "S");
	Connect("R_0", "or_0", "input_0");
	Connect("R_1", "or_1", "input_0");
	Connect("R_2", "or_2", "input_0");
	Connect("R_3", "or_3", "input_0");
	Connect("R_All", "or_0", "input_1");
	Connect("R_All", "or_1", "input_1");
	Connect("R_All", "or_2", "input_1");
	Connect("R_All", "or_3", "input_1");
	
	ChildConnect("or_0", {"sr_latch_0", "R"});
	ChildConnect("or_1", {"sr_latch_1", "R"});
	ChildConnect("or_2", {"sr_latch_2", "R"});
	ChildConnect("or_3", {"sr_latch_3", "R"});
	
	ChildConnect("sr_latch_0", {"Out", "parent", "Out_0"});
	ChildConnect("sr_latch_1", {"Out", "parent", "Out_1"});
	ChildConnect("sr_latch_2", {"Out", "parent", "Out_2"});
	ChildConnect("sr_latch_3", {"Out", "parent", "Out_3"});
}
