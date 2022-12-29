#include "c_sim.hpp"					// Core simulator functionality
#include "devices.h"				// Four_Bit_Counter Device

int main () {
	bool monitor_on = false;
	bool print_probe_samples = true;

	// Set the desired bit-width of the counter here.
	int counter_width = 18;
	
	// Instantiate the top-level Device (the Simulation).
	Simulation sim("test_sim");
	
	// Add a 4-bit counter device.
	sim.AddComponent(new N_Bit_Counter_AIO(&sim, "test_counter", counter_width, monitor_on, {{"run", true}}));
	
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
	
	// Add two Probes and connect them to the counter's outputs and clk input.
	sim.AddProbe("counter_outputs", "test_sim:test_counter", out_pins, "clock_0");
	
	// Run the simulation for 33 ticks.
	sim.Run(100, true, print_probe_samples);
	
	return 0;
}

