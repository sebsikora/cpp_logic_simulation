#include "c_core.h"			// Core simulator functionality
#include "simple_ram.h"		// SimpleRam MagicDevice

int main () {
	// Verbosity flags. Set verbose & monitor_on equal to true to display verbose simulation output in the console.
	bool verbose = true;
	bool monitor_on = false;
	bool print_probe_samples = true;
	
	// Instantiate the top-level Device (the Simulation).
	Simulation* sim = new Simulation("test_sim", 10, verbose);
	
	// Add the SimpleRam MagicDevice and call stabilise.
	sim->AddComponent(new SimpleRam(sim, "test_ram", 8, 8, monitor_on, {{"write", true}, {"read", false}}));
	sim->Stabilise();
	
	// Add a Clock and connect it to the clk input on the register.
	// The Clock output will be a repeating pattern of false, true, false, true, etc, starting on false on the first tick.
	sim->AddClock("clock_0", {false, true}, monitor_on);
	sim->ClockConnect("clock_0", "test_ram", "clk");
	
	// Add Probes connected to the clk, read and write control inputs of the register, to it's address and data input pins, and data output pins.
	sim->AddProbe("ram_address_in", "test_sim:test_ram", {"a_0", "a_1", "a_2", "a_3", "a_4", "a_5", "a_6", "a_7"}, "clock_0");
	sim->AddProbe("ram_data_in", "test_sim:test_ram", {"d_in_0", "d_in_1", "d_in_2", "d_in_3", "d_in_4", "d_in_5", "d_in_6", "d_in_7"}, "clock_0");
	sim->AddProbe("ram_data_out", "test_sim:test_ram", {"d_out_0", "d_out_1", "d_out_2", "d_out_3", "d_out_4", "d_out_5", "d_out_6", "d_out_7"}, "clock_0");
	sim->AddProbe("ram_clk_in", "test_sim:test_ram", {"clk"}, "clock_0");
	sim->AddProbe("ram_read_in", "test_sim:test_ram", {"read"}, "clock_0");
	sim->AddProbe("ram_write_in", "test_sim:test_ram", {"write"}, "clock_0");
	
	// Write a word to memory address 0.
	//sim->ChildSet("test_ram", "write", true);
	//sim->ChildSet("test_ram", "read", false);
	// Set address pins.
	sim->ChildSet("test_ram", "a_0", false);
	sim->ChildSet("test_ram", "a_1", false);
	sim->ChildSet("test_ram", "a_2", false);
	sim->ChildSet("test_ram", "a_3", false);
	sim->ChildSet("test_ram", "a_4", false);
	sim->ChildSet("test_ram", "a_5", false);
	sim->ChildSet("test_ram", "a_6", false);
	sim->ChildSet("test_ram", "a_7", false);
	// Set data pins.
	sim->ChildSet("test_ram", "d_in_0", true);
	sim->ChildSet("test_ram", "d_in_1", true);
	sim->ChildSet("test_ram", "d_in_2", true);
	sim->ChildSet("test_ram", "d_in_3", true);
	sim->ChildSet("test_ram", "d_in_4", false);
	sim->ChildSet("test_ram", "d_in_5", false);
	sim->ChildSet("test_ram", "d_in_6", false);
	sim->ChildSet("test_ram", "d_in_7", false);
	// Run the simulation for three ticks to get the T -> F clock transition from tick 1 to 2 to write the word.
	sim->Run(3, true);
	
	// Change address and write a different word.
	sim->ChildSet("test_ram", "a_0", true);
	sim->ChildSet("test_ram", "a_1", false);
	sim->ChildSet("test_ram", "a_2", false);
	sim->ChildSet("test_ram", "a_3", false);
	sim->ChildSet("test_ram", "a_4", false);
	sim->ChildSet("test_ram", "a_5", false);
	sim->ChildSet("test_ram", "a_6", false);
	sim->ChildSet("test_ram", "a_7", false);
	// Set data pins.
	sim->ChildSet("test_ram", "d_in_0", false);
	sim->ChildSet("test_ram", "d_in_1", false);
	sim->ChildSet("test_ram", "d_in_2", false);
	sim->ChildSet("test_ram", "d_in_3", false);
	sim->ChildSet("test_ram", "d_in_4", true);
	sim->ChildSet("test_ram", "d_in_5", true);
	sim->ChildSet("test_ram", "d_in_6", true);
	sim->ChildSet("test_ram", "d_in_7", true);
	// Run the simulation for two ticks to get the T -> F clock transition to write the word.
	sim->Run(2, false);
	
	// Change address and read back the first word.
	sim->ChildSet("test_ram", "write", false);
	sim->ChildSet("test_ram", "read", true);
	sim->ChildSet("test_ram", "a_0", false);
	sim->ChildSet("test_ram", "a_1", false);
	sim->ChildSet("test_ram", "a_2", false);
	sim->ChildSet("test_ram", "a_3", false);
	sim->ChildSet("test_ram", "a_4", false);
	sim->ChildSet("test_ram", "a_5", false);
	sim->ChildSet("test_ram", "a_6", false);
	sim->ChildSet("test_ram", "a_7", false);
	// Run the simulation for two ticks to get the T -> F clock transition to read the word.
	sim->Run(2, false);
	
	// Change address and read back the second word.
	sim->ChildSet("test_ram", "a_0", true);
	sim->ChildSet("test_ram", "a_1", false);
	sim->ChildSet("test_ram", "a_2", false);
	sim->ChildSet("test_ram", "a_3", false);
	sim->ChildSet("test_ram", "a_4", false);
	sim->ChildSet("test_ram", "a_5", false);
	sim->ChildSet("test_ram", "a_6", false);
	sim->ChildSet("test_ram", "a_7", false);
	// Run the simulation for two ticks to get the T -> F clock transition to read the word.
	sim->Run(2, false, verbose, print_probe_samples);
	
	delete sim;
	
	return 0;
}

//~sim.ChildConnect("test_ram", {"d_out_7", "parent", "__ALL_STOP__"});
