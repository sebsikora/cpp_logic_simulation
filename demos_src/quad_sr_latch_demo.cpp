// quad_sr_latch_demo.cpp

#include "c_core.h"           // Core simulator functionality
#include "quad_sr_latch.h"    // Our new Quad_SR_latch device.

int main () {
	bool verbose = false;
	Simulation sim("test_sim", verbose);
	
	// Add quad SR latch device to the top-level simulation.
	//
	bool monitor_on = true;
	sim.AddComponent(new Quad_SR_Latch(&sim, "quad_sr_latch", monitor_on, {{"S_0", false}, {"S_1", false},
	                 {"S_2", false}, {"S_3", false}, {"R_0", false}, {"R_1", false}, {"R_2", false},{"R_3", false}, {"R_All", false}}));
	
	sim.Stabilise();          // Settle initial device internal and external states.
	
	// 'Reset 0'.
	sim.ChildSet("quad_sr_latch", "R_0", true);
	sim.ChildSet("quad_sr_latch", "R_0", false);
	
	// 'Reset 1'.
	sim.ChildSet("quad_sr_latch", "R_1", true);
	sim.ChildSet("quad_sr_latch", "R_1", false);
	
	// 'Reset 2'.
	sim.ChildSet("quad_sr_latch", "R_2", true);
	sim.ChildSet("quad_sr_latch", "R_2", false);
	
	// 'Reset 3'.
	sim.ChildSet("quad_sr_latch", "R_3", true);
	sim.ChildSet("quad_sr_latch", "R_3", false);
	
	// 'Set 0'.
	sim.ChildSet("quad_sr_latch", "S_0", true);
	sim.ChildSet("quad_sr_latch", "S_0", false);
	
	// 'Set 1'.
	sim.ChildSet("quad_sr_latch", "S_1", true);
	sim.ChildSet("quad_sr_latch", "S_1", false);
	
	// 'Set 2'.
	sim.ChildSet("quad_sr_latch", "S_2", true);
	sim.ChildSet("quad_sr_latch", "S_2", false);
	
	// 'Set 3'.
	sim.ChildSet("quad_sr_latch", "S_3", true);
	sim.ChildSet("quad_sr_latch", "S_3", false);
	
	// 'Reset All'.
	sim.ChildSet("quad_sr_latch", "R_All", true);
	sim.ChildSet("quad_sr_latch", "R_All", false);
	
	return 0;
}
