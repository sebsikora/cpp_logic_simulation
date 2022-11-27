#include "c_sim.hpp"
#include "devices.h"		// JK flip-flop Device

int main () {
	bool print_probe_samples = true;
	
	// Instantiate the top-level Device (the Simulation).
	Simulation sim("test_sim");
	
	// Add a jk flip-flop Device.
	sim.AddComponent(new JK_FF(&sim, "test_ff", true, {{"j", false}, {"k", false}}));
	
	// We could leave out the default in pin state vector argument, and instead connect 'j' and 'k' in pins
	// to the Simulation's 'true' hidden in pin...
	//~sim.Connect("true", "test_ff", "j");
	//~sim.Connect("true", "test_ff", "k");
	
	// Once we have added all our devices, call the simulation's Stabilise() method to finish setup.
	sim.Stabilise();

	//~// Add a Clock and connect it to the clk input on the jk flip-flop.
	//~// The Clock output will be a repeating pattern of false, true, false, true, etc, starting on false on the first tick.
	sim.AddClock("clock_0", {false, true}, true);
	sim.ClockConnect("clock_0", "test_ff", "clk");
	
	//~// Add two Probes and connect them to the jk flip-flop's outputs and clk input.
	//~// Note - we need to use the target Component's 'full name' to connect a Probe.
	sim.AddProbe("flip-flop outputs", "test_sim:test_ff", {"q", "not_q"}, "clock_0");
	sim.AddProbe("flip-flop clk input", "test_sim:test_ff", {"clk"}, "clock_0");

	// ...or we could set the pin states manually.
	sim.ChildSet("test_ff", "j", true);
	sim.ChildSet("test_ff", "k", true);
	
	//~// Run the simulation for ten ticks.
	sim.Run(6, true, print_probe_samples);
	
	return 0;
}
