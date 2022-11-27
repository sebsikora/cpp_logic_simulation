// sr_latch.cpp

#include "c_structs.hpp"
#include "c_gate.hpp"
#include "c_device.hpp"

#include "sr_latch.h"       // Our new SR_latch device.

SR_Latch::SR_Latch(Device* parent_device_pointer, std::string name, bool monitor_on, std::vector<StateDescriptor> in_pin_default_states) 
 : Device(parent_device_pointer, name, "sr_latch", {"S", "R"}, {"Out"}, monitor_on, in_pin_default_states) {
	// Following base class constructor (Device), we call the below overridden Build() method to populate the
	// specific device, then we call the base Stabilise() method to configure initial internal device component state.
	Build();
	Stabilise();
}

void SR_Latch::Build() {
	// Instantiate gates.
	//
	AddGate("or_0", "or", {"input_0", "input_1"}, false);   // monitor_on flag can be omitted,
	AddGate("and_0", "and", {"input_0", "input_1"});        // defaults to false.
	AddGate("not_0", "not");

	// Child gate interconnections.
	//
	ChildConnect("or_0", {"and_0", "input_0"});
	ChildConnect("not_0", {"and_0", "input_1"});
	ChildConnect("and_0", {"or_0", "input_0"});

	// Connections from parent device in pins & to parent device out pin.
	//
	Connect("S", "or_0", "input_1");                        // device's Connect() member function used
	Connect("R", "not_0");                                  // to connect parent in pin to child in pin.
	ChildConnect("and_0", {"parent", "Out"});               // ChildConnect() member function used to connect
	                                                        // child out pin to parent out pin.
}
