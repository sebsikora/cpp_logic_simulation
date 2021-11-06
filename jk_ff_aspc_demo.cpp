#include "c_core.h"			// Core simulator functionality
#include "devices.h"		// JK flip-flop Device

int main () {
	// Verbosity flags. Set verbose & monitor_on equal to true to display verbose simulation output in the console.
	bool verbose = false;
	bool monitor_on = true;
	bool print_probe_samples = true;
	
	// Instantiate the top-level Device (the Simulation).
	Simulation* sim = new Simulation("test_sim", verbose);
	
	// Add a jk flip-flop Device.
	sim->AddComponent(new JK_FF_ASPC(sim, "test_ff", monitor_on, {{"j", false}, {"k", false}, {"not_p", true}, {"not_c", true}}));
	
	// Once we have added all our devices, call the simulation's Stabilise() method to finish setup.
	sim->Stabilise();
	
	// Add a Clock and connect it to the clk input on the jk flip-flop.
	// The Clock output will be a repeating pattern of false, true, false, true, etc, starting on false on the first tick.
	sim->AddClock("clock_0", {false, true}, monitor_on);
	sim->ClockConnect("clock_0", "test_ff", "clk");

	// Add the jk flip-flop to the probable Devices list.
	sim->ChildMakeProbable("test_ff");
	
	// Add two Probes and connect them to the jk flip-flop's outputs and clk input.
	sim->AddProbe("flip-flop outputs", "test_sim:test_ff", {"q", "not_q"}, "clock_0");
	sim->AddProbe("flip-flop clk input", "test_sim:test_ff", {"clk"}, "clock_0");
	
	sim->Run(1, true, verbose, false);
	
	sim->ChildSet("test_ff", "not_c", false);
	sim->ChildSet("test_ff", "not_c", true);
	sim->Run(1, false, verbose, false);
	
	sim->ChildSet("test_ff", "not_p", false);
	sim->ChildSet("test_ff", "not_p", true);
	sim->Run(1, false, verbose, false);
	// Run the simulation for ten ticks.
	
	sim->ChildSet("test_ff", "j", true);
	sim->ChildSet("test_ff", "k", true);
	sim->Run(10, false, verbose, print_probe_samples);

	delete sim;
	
	return 0;
}
