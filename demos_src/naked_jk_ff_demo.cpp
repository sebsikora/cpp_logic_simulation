#include "c_core.h"					// Core simulator functionality

int main () {
	// Verbosity flags. Set verbose & monitor_on equal to true to display verbose simulation output in the console.
	bool verbose = false;
	bool monitor_on = true;
	bool print_probe_samples = true;
	
	// Instantiate the top-level Device (the Simulation).
	Simulation sim("test_sim", verbose);
	
	// Instantiate components.
	// Can do it by directly instantiating the object.
	sim.AddComponent(new Gate(&sim, "nand_1", "nand", {"input_0", "input_1", "input_2"}, false));
	// Or to create Gates we can use a parent Device member function.
	sim.AddGate("nand_2", "nand", {"input_0", "input_1", "input_2"}, false);
	sim.AddGate("nand_3", "nand", {"input_0", "input_1"}, false);
	sim.AddGate("nand_4", "nand", {"input_0", "input_1"}, false);
	sim.AddGate("nand_5", "nand", {"input_0", "input_1"}, false);
	sim.AddGate("nand_6", "nand", {"input_0", "input_1"}, false);
	sim.AddGate("nand_7", "nand", {"input_0", "input_1"}, monitor_on);
	sim.AddGate("nand_8", "nand", {"input_0", "input_1"}, monitor_on);
	sim.AddGate("not_1", "not");	// For not Gates we can leave off the in pins vector, it will be replaced by a single "input".
	
	// Interconnect components.
	sim.ChildConnect("nand_1", {"nand_3", "input_0"});
	sim.ChildConnect("nand_2", {"nand_4", "input_0"});
	sim.ChildConnect("nand_3", {"nand_4", "input_1"});
	sim.ChildConnect("nand_4", {"nand_3", "input_1"});
	
	sim.ChildConnect("nand_3", {"nand_5", "input_0"});
	sim.ChildConnect("nand_4", {"nand_6", "input_0"});
	
	sim.ChildConnect("not_1", {"nand_5", "input_1"});
	sim.ChildConnect("not_1", {"nand_6", "input_1"});
	
	sim.ChildConnect("nand_5", {"nand_7", "input_0"});
	sim.ChildConnect("nand_6", {"nand_8", "input_0"});
	sim.ChildConnect("nand_7", {"nand_8", "input_1"});
	sim.ChildConnect("nand_8", {"nand_7", "input_1"});
	
	sim.ChildConnect("nand_7", {"nand_2", "input_1"});
	sim.ChildConnect("nand_8", {"nand_1", "input_1"});
	
	// Tie the 'j' and 'k' in pins to the Simulation's 'true' hidden in pin.
	sim.Connect("true", "nand_1", "input_0");
	sim.Connect("true", "nand_2", "input_0");
	
	// Once we have added all our devices, call the simulation's Stabilise() method to finish setup.
	sim.Stabilise();
	
	// Add a clock to drive the circuit.
	sim.AddClock("clock_0", {false, true}, monitor_on);
	
	// Connect the clock.
	sim.ClockConnect("clock_0", "nand_1", "input_2");
	sim.ClockConnect("clock_0", "nand_2", "input_2");
	sim.ClockConnect("clock_0", "not_1", "input");
	
	sim.AddProbe("q", "test_sim:nand_7", {"output"}, "clock_0");
	sim.AddProbe("not_q", "test_sim:nand_8", {"output"}, "clock_0");
	
	// Run the simulation for 8 ticks. We should see the two probed out pins opposite
	// and toggling every other tick on the true->false clock transition.
	sim.Run(8, true, verbose, print_probe_samples);
		
	return 0;
}

