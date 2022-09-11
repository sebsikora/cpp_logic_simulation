// sr_latch_demo.cpp

#include "c_sim.hpp"

int main () {
	bool verbose = false;
	bool monitor_on = true;
	
	Simulation sim("test_sim", verbose);
	
	sim.AddGate("or_0", "or", {"input_0", "input_1"}, monitor_on);
	sim.AddGate("and_0", "and", {"input_0", "input_1"}, monitor_on);
	sim.AddGate("not_0", "not", monitor_on);
	
	sim.Connect("false", "or_0", "input_1");
	sim.Connect("false", "not_0");
	
	sim.ChildConnect("or_0", {"and_0", "input_0"});
	sim.ChildConnect("not_0", {"and_0", "input_1"});
	sim.ChildConnect("and_0", {"or_0", "input_0"});
	
	sim.Stabilise();
	
	// S input is or_0:input_1, R input is not_0:input
	// Output is and_0:output
	
	// 'Set'.
	sim.ChildSet("or_0", "input_1", true);
	sim.ChildSet("or_0", "input_1", false);
	
	// 'Reset'.
	sim.ChildSet("not_0", "input", true);
	sim.ChildSet("not_0", "input", false);
	
	// 'Set'.
	sim.ChildSet("or_0", "input_1", true);
	sim.ChildSet("or_0", "input_1", false);
	
	// 'Reset'.
	sim.ChildSet("not_0", "input", true);
	sim.ChildSet("not_0", "input", false);
	
	return 0;
}
	
	
	
