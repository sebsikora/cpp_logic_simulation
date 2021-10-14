// This runs 50K ticks (25K up-counts) on the simple 4 bit counter, number_of_runs times, timing with millisecond precision.

#include <chrono>
#include <iostream>
#include <string>

#include "c_core.h"			// Core simulator functionality
#include "devices.h"		// Four_Bit_Counter Device

int main () {
	// Verbosity flags. Set verbose & monitor_on equal to true to display verbose simulation output in the console.
	bool verbose = false;
	bool monitor_on = false;
	bool print_probe_samples = false;

	int number_of_runs = 10;
	
	// Instantiate the top-level Device (the Simulation).
	Simulation sim = Simulation("test_sim", 10, verbose);
	
	// Add a 4-bit counter device.
	sim.AddComponent(new Four_Bit_Counter(&sim, "test_counter", monitor_on, {{"run", true}}));
	
	// Once we have added all our devices, call the simulation's Stabilise() method to finish setup.
	sim.Stabilise();
	
	// Add a Clock and connect it to the clk input on the counter.
	// The Clock output will be a repeating pattern of false, true, false, true, etc, starting on false on the first tick.
	sim.AddClock("clock_0", {false, true}, monitor_on);
	sim.ClockConnect("clock_0", "test_counter", "clk");
	
	// Add two Probes and connect them to the counter's outputs and clk input.
	sim.AddProbe("counter_outputs", "test_sim:test_counter", {"q_0", "q_1", "q_2", "q_3"}, "clock_0");
	sim.AddProbe("clk_input", "test_sim:test_counter", {"clk"}, "clock_0");
	
	// Set the counter's run input to high (true).
	//sim.ChildSet("test_counter", "run", true);
	
	// Run the simulation for 33 ticks.
	std::vector<int> run_times = {};
	using std::chrono::high_resolution_clock;
	for (int i = 0; i < number_of_runs; i ++) {
		std::cout << " LOOPED RUN # " << std::to_string(i) << std::endl;
		auto t1 = high_resolution_clock::now();
		sim.Run(50000, true, verbose, print_probe_samples);
		auto t2 = high_resolution_clock::now();
		std::chrono::duration<double, std::milli> ms_int = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
		run_times.push_back(ms_int.count());
		std::cout << "Run " << std::to_string(i) << " took " << std::to_string(ms_int.count()) << " ms" << std::endl;
	}
	int index = 0;
	for (const auto& run_time: run_times) {
		std::cout << std::to_string(index) << "," << std::to_string(run_time) << std::endl;
		index ++;
	}
	return 0;
}

