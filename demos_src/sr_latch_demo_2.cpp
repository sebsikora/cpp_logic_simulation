// sr_latch_demo_2.cpp

#include "c_core.h"           // Core simulator functionality
#include "sr_latch.h"         // Our new SR_latch device.

int main () {
	bool verbose = false;
	Simulation sim("test_sim", verbose);

	// Add an SR latch device to the top-level simulation.
	//
	bool monitor_on = true;
	sim.AddComponent(new SR_Latch(&sim, "sr_latch", monitor_on, {{"S", false}, {"R", false}}));
	
	sim.Stabilise();          // Settle initial device internal and external states.
	
	// 'Set'.
	sim.ChildSet("sr_latch", "S", true);
	sim.ChildSet("sr_latch", "S", false);
	
	// 'Reset'.
	sim.ChildSet("sr_latch", "R", true);
	sim.ChildSet("sr_latch", "R", false);
	
	// 'Set'.
	sim.ChildSet("sr_latch", "S", true);
	sim.ChildSet("sr_latch", "S", false);
	
	// 'Reset'.
	sim.ChildSet("sr_latch", "R", true);
	sim.ChildSet("sr_latch", "R", false);
	
	return 0;
}
