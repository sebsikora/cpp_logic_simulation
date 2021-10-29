#include "c_core.h"			// Core simulator functionality
#include "simple_rom.h"		// SimpleRom MagicDevice

int main () {
	// Verbosity flags. Set verbose & monitor_on equal to true to display verbose simulation output in the console.
	bool verbose = true;
	bool monitor_on = false;
	bool print_probe_samples = true;
	
	// Instantiate the top-level Device (the Simulation).
	Simulation* sim = new Simulation("test_sim", 10, verbose);
	
	// Add the SimpleRom MagicDevice and call stabilise.
	sim->AddComponent(new SimpleRom(sim, "test_rom", "data.txt", monitor_on, {{"read", true}}));
	sim->Stabilise();
	
	// Add a Clock and connect it to the clk input on the register.
	// The Clock output will be a repeating pattern of false, true, false, true, etc, starting on false on the first tick.
	sim->AddClock("clock_0", {false, true}, monitor_on);
	sim->ClockConnect("clock_0", "test_rom", "clk");
	
	sim->ChildMakeProbable("test_rom");
	
	// Add Probes connected to the clk & read control inputs of the register, to it's address input pins and data output pins.
	sim->AddProbe("rom_address_in", "test_sim:test_rom", {"a_0", "a_1", "a_2", "a_3", "a_4", "a_5", "a_6", "a_7"}, "clock_0");
	sim->AddProbe("rom_data_out", "test_sim:test_rom", {"d_0", "d_1", "d_2", "d_3", "d_4", "d_5", "d_6", "d_7"}, "clock_0");
	sim->AddProbe("rom_clk_in", "test_sim:test_rom", {"clk"}, "clock_0");
	sim->AddProbe("rom_read_in", "test_sim:test_rom", {"read"}, "clock_0");
	
	// Set the *read* control input high and clear the address input pins.
	//sim->ChildSet("test_rom", "read", true);
	sim->ChildSet("test_rom", "a_0", false);
	sim->ChildSet("test_rom", "a_1", false);
	sim->ChildSet("test_rom", "a_2", false);
	sim->ChildSet("test_rom", "a_3", false);
	sim->ChildSet("test_rom", "a_4", false);
	sim->ChildSet("test_rom", "a_5", false);
	sim->ChildSet("test_rom", "a_6", false);
	sim->ChildSet("test_rom", "a_7", false);
	// Run the simulation for three ticks to get the first high to low clock transition.
	sim->Run(3, true);
	
	// Increment the address and run two ticks to read another word.
	sim->ChildSet("test_rom", "a_0", true);
	sim->ChildSet("test_rom", "a_1", false);
	sim->ChildSet("test_rom", "a_2", false);
	sim->ChildSet("test_rom", "a_3", false);
	sim->ChildSet("test_rom", "a_4", false);
	sim->ChildSet("test_rom", "a_5", false);
	sim->ChildSet("test_rom", "a_6", false);
	sim->ChildSet("test_rom", "a_7", false);
	sim->Run(2, false);
	
	// Increment the address and run two ticks to read another word.
	sim->ChildSet("test_rom", "a_0", false);
	sim->ChildSet("test_rom", "a_1", true);
	sim->ChildSet("test_rom", "a_2", false);
	sim->ChildSet("test_rom", "a_3", false);
	sim->ChildSet("test_rom", "a_4", false);
	sim->ChildSet("test_rom", "a_5", false);
	sim->ChildSet("test_rom", "a_6", false);
	sim->ChildSet("test_rom", "a_7", false);
	sim->Run(2, false);
	
	// Increment the address and run two ticks to read another word.
	sim->ChildSet("test_rom", "a_0", true);
	sim->ChildSet("test_rom", "a_1", true);
	sim->ChildSet("test_rom", "a_2", false);
	sim->ChildSet("test_rom", "a_3", false);
	sim->ChildSet("test_rom", "a_4", false);
	sim->ChildSet("test_rom", "a_5", false);
	sim->ChildSet("test_rom", "a_6", false);
	sim->ChildSet("test_rom", "a_7", false);
	sim->Run(2, false);
	
	// Increment the address and run two ticks to read another word.
	sim->ChildSet("test_rom", "a_0", false);
	sim->ChildSet("test_rom", "a_1", false);
	sim->ChildSet("test_rom", "a_2", true);
	sim->ChildSet("test_rom", "a_3", false);
	sim->ChildSet("test_rom", "a_4", false);
	sim->ChildSet("test_rom", "a_5", false);
	sim->ChildSet("test_rom", "a_6", false);
	sim->ChildSet("test_rom", "a_7", false);
	sim->Run(2, false);
	
	// Set the *read* control input low and change the address. 
	sim->ChildSet("test_rom", "read", false);
	sim->ChildSet("test_rom", "a_0", true);
	sim->ChildSet("test_rom", "a_1", true);
	sim->ChildSet("test_rom", "a_2", true);
	sim->ChildSet("test_rom", "a_3", true);
	sim->ChildSet("test_rom", "a_4", true);
	sim->ChildSet("test_rom", "a_5", true);
	sim->ChildSet("test_rom", "a_6", true);
	sim->ChildSet("test_rom", "a_7", true);
	sim->Run(2, false);
	
	// Bring the *read* input high again and run the sim two ticks to read the final empty word in the memory.
	sim->ChildSet("test_rom", "read", true);
	sim->Run(2, false, false, print_probe_samples);
	
	delete sim;
	
	return 0;
}

