#include <iostream>

#include "c_core.h"
#include "devices.h"

int main () {
	bool verbose = false;
	bool monitor_on = true;
	
	Simulation* sim = new Simulation("test_sim", 10, verbose);
	
	sim->AddComponent(new NxOne_Bit_Mux(sim, "test_mux", 3, monitor_on));
	sim->Stabilise();
	
	sim->ChildMakeProbable("test_mux");
	
	// Add a Clock and connect it to the clk input on the register.
	// The Clock output will be a repeating pattern of false, true, false, true, etc, starting on false on the first tick.
	sim->AddClock("clock_0", {false, true}, monitor_on);
	
	sim->AddProbe("sel_in", "test_sim:test_mux", {"sel_in_0", "sel_in_1", "sel_in_2"}, "clock_0");
	sim->AddProbe("d_in", "test_sim:test_mux", {"d_in_0", "d_in_1", "d_in_2"}, "clock_0");
	sim->AddProbe("d_out", "test_sim:test_mux", {"d_out"}, "clock_0");
	
	sim->ChildSet("test_mux", "d_in_0", true);
	sim->ChildSet("test_mux", "d_in_1", false);
	sim->ChildSet("test_mux", "d_in_2", true);
	sim->ChildSet("test_mux", "sel_in_0", true);
	sim->ChildSet("test_mux", "sel_in_1", false);
	sim->ChildSet("test_mux", "sel_in_2", false);
	sim->Run(1, true, false, false);
	
	sim->ChildSet("test_mux", "sel_in_0", false);
	sim->ChildSet("test_mux", "sel_in_1", true);
	sim->ChildSet("test_mux", "sel_in_2", false);
	sim->Run(1, false, false, false);
	
	sim->ChildSet("test_mux", "sel_in_0", false);
	sim->ChildSet("test_mux", "sel_in_1", false);
	sim->ChildSet("test_mux", "sel_in_2", true);
	sim->Run(1, false, false, true);
	
	delete sim;
	
	return 0;
}



