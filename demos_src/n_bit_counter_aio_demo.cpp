#include "c_core.h"					// Core simulator functionality
#include "devices.h"				// Four_Bit_Counter Device

int main () {
	// Verbosity flags. Set verbose & monitor_on equal to true to display verbose simulation output in the console.
	bool verbose = false;
	bool monitor_on = false;
	bool print_probe_samples = true;
	
	// Instantiate the top-level Device (the Simulation).
	Simulation sim("test_sim", verbose);
	
	// Add a 4-bit counter device.
	sim.AddComponent(new N_Bit_Counter_AIO(&sim, "test_counter", 4, monitor_on, {{"run", true}}));
	
	// Once we have added all our devices, call the simulation's Stabilise() method to finish setup.
	sim.Stabilise();
	
	// Add a Clock and connect it to the clk input on the counter.
	// The Clock output will be a repeating pattern of false, true, false, true, etc, starting on false on the first tick.
	sim.AddClock("clock_0", {false, true}, monitor_on);
	sim.ClockConnect("clock_0", "test_counter", "clk");
	
	// Add two Probes and connect them to the counter's outputs and clk input.
	sim.AddProbe("counter_outputs", "test_sim:test_counter", {"q_0", "q_1", "q_2", "q_3"}, "clock_0");
	
	// Run the simulation for 33 ticks.
	sim.Run(33, true, verbose, print_probe_samples);
	
	return 0;
}

