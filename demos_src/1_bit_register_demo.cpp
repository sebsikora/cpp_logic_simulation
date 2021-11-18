#include "c_core.h"			// Core simulator functionality
#include "devices.h"		// One_Bit_Register Device

int main () {
	// Verbosity flags. Set verbose & monitor_on equal to true to display verbose simulation output in the console.
	bool verbose = false;
	bool monitor_on = false;
	bool print_probe_samples = true;
	
	// Instantiate the top-level Device (the Simulation).
	Simulation sim("test_sim", verbose);
	
	// Add a 4-bit counter device.
	sim.AddComponent(new One_Bit_Register(&sim, "test_reg", monitor_on, {{"clr", true}, {"load", false}, {"d_in", false}}));

	// Once we have added all our devices, call the simulation's Stabilise() method to finish setup.
	sim.Stabilise();
	
	// Add a Clock and connect it to the clk input on the register.
	// The Clock output will be a repeating pattern of false, true, false, true, etc, starting on false on the first tick.
	sim.AddClock("clock_0", {false, true}, monitor_on);
	sim.ClockConnect("clock_0", "test_reg", "clk");
	
	// Add Probes connected to the clk, load and clear inputs of the register and one to it's data output.
	sim.AddProbe("reg_clk_input", "test_sim:test_reg", {"clk"}, "clock_0");
	sim.AddProbe("reg_clr_input", "test_sim:test_reg", {"clr"}, "clock_0");
	sim.AddProbe("reg_load_input", "test_sim:test_reg", {"load"}, "clock_0");
	sim.AddProbe("reg_data_output", "test_sim:test_reg", {"d_out"}, "clock_0");
	
	//sim.ChildSet("test_reg", "d_in", false);
	//sim.ChildSet("test_reg", "clr", true);
	//sim.ChildSet("test_reg", "load", false);
	sim.Run(3, true, verbose, false);
	
	sim.ChildSet("test_reg", "d_in", true);
	sim.ChildSet("test_reg", "load", true);
	sim.ChildSet("test_reg", "clr", false);
	sim.Run(2, false, verbose, false);
	
	sim.ChildSet("test_reg", "d_in", false);
	sim.ChildSet("test_reg", "load", false);
	sim.Run(4, false, verbose, false);
	
	sim.ChildSet("test_reg", "clr", true);
	sim.Run(2, false, verbose, false);
	
	sim.ChildSet("test_reg", "clr", false);
	sim.ChildSet("test_reg", "d_in", true);
	sim.Run(2, false, verbose, print_probe_samples);
	
	return 0;
}

