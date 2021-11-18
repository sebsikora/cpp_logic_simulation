// n_bit_sr_latch_demo.cpp
#include <vector>
#include <string>

#include "c_core.h"           // Core simulator functionality
#include "n_bit_sr_latch.h"   // Our new Quad_SR_latch device.

int main () {
	bool verbose = false;
	Simulation sim("test_sim", verbose);
	
	int latch_count = 4;
	
	// Make a vector of default in pin states.
	std::vector<state_descriptor> in_pin_default_states = {};
	for (int latch_index = 0; latch_index < latch_count; latch_index ++) {
		in_pin_default_states.push_back({"S_" + std::to_string(latch_index), false});
		in_pin_default_states.push_back({"R_" + std::to_string(latch_index), false});
	}
	in_pin_default_states.push_back({"R_All", false});
	
	// Add an n-bit SR latch device to the top-level simulation.
	//
	bool monitor_on = true;
	sim.AddComponent(new N_Bit_SR_Latch(&sim, "n_bit_sr_latch", latch_count, monitor_on, in_pin_default_states));
	
	sim.Stabilise();          // Settle initial device internal and external states.
	
	// Reset latches one at a time.
	for (int latch_index = 0; latch_index < latch_count; latch_index ++) {
		sim.ChildSet("n_bit_sr_latch", "R_" + std::to_string(latch_index), true);
		sim.ChildSet("n_bit_sr_latch", "R_" + std::to_string(latch_index), false);
	}
	
	// Set latches one at a time.
	for (int latch_index = 0; latch_index < latch_count; latch_index ++) {
		sim.ChildSet("n_bit_sr_latch", "S_" + std::to_string(latch_index), true);
		sim.ChildSet("n_bit_sr_latch", "S_" + std::to_string(latch_index), false);
	}
	
	// 'Reset All'.
	sim.ChildSet("n_bit_sr_latch", "R_All", true);
	sim.ChildSet("n_bit_sr_latch", "R_All", false);
	
	return 0;
}
