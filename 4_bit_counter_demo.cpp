#include "c_core.h"					// Core simulator functionality
#include "devices.h"				// Four_Bit_Counter Device

int main () {
	// Verbosity flags. Set verbose & monitor_on equal to true to display verbose simulation output in the console.
	bool verbose = false;
	bool monitor_on = false;
	bool print_probe_samples = true;
	
	// Instantiate the top-level Device (the Simulation).
	Simulation* sim = new Simulation("test_sim", 10, verbose);
	
	// Add a 4-bit counter device.
	sim->AddComponent(new Four_Bit_Counter(sim, "test_counter", monitor_on, {{"run", true}}));
	
	// Add a Clock and connect it to the clk input on the counter.
	// The Clock output will be a repeating pattern of false, true, false, true, etc, starting on false on the first tick.
	sim->AddClock("clock_0", {true, false}, monitor_on);
	sim->ClockConnect("clock_0", "test_counter", "clk");
	
	// Once we have added all our devices, call the simulation's Stabilise() method to finish setup.
	sim->Stabilise();
	
	sim->ChildMakeProbable("test_counter");
	
	// Add two Probes and connect them to the counter's outputs and clk input.
	sim->AddProbe("clk_input", "test_sim:test_counter", {"clk"}, "clock_0");
	sim->AddProbe("counter_outputs", "test_sim:test_counter", {"q_0", "q_1", "q_2", "q_3"}, "clock_0");
	
	//~// Run the simulation for 33 ticks.
	sim->Run(16, true, verbose, false);
	sim->ChildSet("test_counter", "run", false);
	sim->Run(4, false, verbose, false);
	sim->ChildSet("test_counter", "run", true);
	sim->Run(16, false, verbose, print_probe_samples);
	
	delete sim;
	
	return 0;
}

