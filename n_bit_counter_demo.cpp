#include "c_core.h"			// Core simulator functionality
#include "devices.h"		// N_Bit_Counter Device

int main () {
	// Verbosity flags. Set verbose & monitor_on equal to true to display verbose simulation output in the console.
	bool verbose = false;
	bool monitor_on = false;
	bool print_probe_samples = true;
	
	// Set the desired bit-width of the counter here.
	int counter_width = 4;
	
	// Instantiate the top-level Device (the Simulation).
	std::string sim_name = "test_sim";
	Simulation sim(sim_name, 10, verbose);
	
	// Add the n-bit counter Device. Note the parameterised counter width.
	sim.AddComponent(new N_Bit_Counter(&sim, "test_counter", counter_width, monitor_on, {{"run", true}}));
	
	// Once we have added all our devices, call the simulation's Stabilise() method to finish setup.
	sim.Stabilise();
	
	// Add a Clock and connect it to the clk input on the counter.
	// The Clock output will be a repeating pattern of false, true, false, true, etc, starting on false on the first tick.
	sim.AddClock("clock_0", {false, true}, monitor_on);
	sim.ClockConnect("clock_0", "test_counter", "clk");
	
	// Programmatically generate the required vector of counter output pin names.
	std::vector<std::string> out_pins = {};
	for (int i = 0; i < counter_width; i ++) {
		std::string this_out_pin = "q_" + std::to_string(i);
		out_pins.push_back(this_out_pin);
	}
	
	// Add a Probe on the counter's output pins.
	sim.AddProbe("counter_out", "test_sim:test_counter", out_pins, "clock_0");
	//~sim.AddProbe("counter_clk_in", "test_sim:test_counter", {"clk"}, "clock_0");
	//~sim.AddProbe("counter_run_in", "test_sim:test_counter", {"run"}, "clock_0");
	
	sim.Run(33, true, verbose, print_probe_samples);
	
	return 0;
}


