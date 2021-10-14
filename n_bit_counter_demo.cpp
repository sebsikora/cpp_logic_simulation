#include "c_core.h"			// Core simulator functionality
#include "devices.h"		// N_Bit_Register Device

int main () {
	// Verbosity flags. Set verbose & monitor_on equal to true to display verbose simulation output in the console.
	bool verbose = false;
	bool monitor_on = false;
	bool print_probe_samples = true;
	
	// Set the desired bit-width of the register here.
	int counter_width = 4;
	
	// Instantiate the top-level Device (the Simulation).
	std::string sim_name = "test_sim";
	Simulation sim = Simulation(sim_name, 10, verbose);
	
	// Add the n-bit register Device. Note the parameterised register width.
	sim.AddComponent(new N_Bit_Counter(&sim, "test_counter", counter_width, monitor_on, {{"run", true}}));
	std::cout << "CHECKPOINT 1" << std::endl;
	// Once we have added all our devices, call the simulation's Stabilise() method to finish setup.
	sim.Stabilise();
	
	// Add a Clock and connect it to the clk input on the register.
	// The Clock output will be a repeating pattern of false, true, false, true, etc, starting on false on the first tick.
	sim.AddClock("clock_0", {false, true}, monitor_on);
	sim.ClockConnect("clock_0", "test_counter", "clk");
	std::cout << "CHECKPOINT 2" << std::endl;
	// Programmatically generate the required vectors of register data input and output pin names.
	// We will need these for attaching probes and driving the data input pins.
	std::vector<std::string> out_pins = {};
	for (int i = 0; i < counter_width; i ++) {
		std::string this_out_pin = "q_" + std::to_string(i);
		out_pins.push_back(this_out_pin);
	}
	
	// Add Probes connected to the clk, load and clear inputs of the register, to it's data input and output pins.
	sim.AddProbe("counter_out", "test_sim:test_counter", out_pins, "clock_0");
	sim.AddProbe("counter_clk_in", "test_sim:test_counter", {"clk"}, "clock_0");
	sim.AddProbe("counter_run_in", "test_sim:test_counter", {"run"}, "clock_0");
	
	sim.Run(40, true, verbose, print_probe_samples);
	
	return 0;
}


