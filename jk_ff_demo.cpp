#include "c_core.h"			// Core simulator functionality
#include "devices.h"		// JK flip-flop Device

int main () {
	// Verbosity flags. Set verbose & monitor_on equal to true to display verbose simulation output in the console.
	bool verbose = false;
	bool monitor_on = false;
	bool print_probe_samples = true;
	
	// Instantiate the top-level Device (the Simulation).
	Simulation sim = Simulation("test_sim", 20, verbose);
	
	// Add a jk flip-flop Device.
	sim.AddComponent(new JK_FF(&sim, "test_ff", monitor_on, {{"j", true}, {"k", true}}));
	//~sim.Connect("true", "test_ff", "j");
	//~sim.Connect("true", "test_ff", "k");
	
	// Once we have added all our devices, call the simulation's Stabilise() method to finish setup.
	sim.Stabilise();
	
	// Add a Clock and connect it to the clk input on the jk flip-flop.
	// The Clock output will be a repeating pattern of false, true, false, true, etc, starting on false on the first tick.
	sim.AddClock("clock_0", {false, true}, monitor_on);
	sim.ClockConnect("clock_0", "test_ff", "clk");
	
	// Add two Probes and connect them to the jk flip-flop's outputs and clk input.
	sim.AddProbe("flip-flop outputs", "test_sim:test_ff", {"q", "not_q"}, "clock_0");
	sim.AddProbe("flip-flop clk input", "test_sim:test_ff", {"clk"}, "clock_0");
	
	//~// Set the jk flip-flop's j & k inputs to high (true).
	//~sim.ChildSet("test_ff", "j", true);
	//~sim.ChildSet("test_ff", "k", true);
	
	// Run the simulation for ten ticks.
	sim.Run(10, true, verbose, print_probe_samples);

	//sim.PrintInputs();
	
	return 0;
}
