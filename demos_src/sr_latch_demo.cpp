// sr_latch_demo.cpp

#include "c_sim.hpp"
#include "c_monitor.hpp"

int main () {
	Simulation sim("test_sim");
	
	sim.AddGate("or_0", "or", {"input_0", "input_1"}, false);
	sim.AddGate("and_0", "and", {"input_0", "input_1"}, true);
	sim.AddGate("not_0", "not", false);

	//~sim.AddComponent(new Monitor(&sim, "sr-latch-output", {"in"}, true));
	
	sim.Connect("false", "or_0", "input_1");
	sim.Connect("false", "not_0");
	
	sim.ChildConnect("or_0", {"and_0", "input_0"});
	sim.ChildConnect("not_0", {"and_0", "input_1"});
	sim.ChildConnect("and_0", {"or_0", "input_0"});
	
	//~sim.ChildConnect("or_0", {"sr-latch-output", "in"});
	
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
	
	
	
