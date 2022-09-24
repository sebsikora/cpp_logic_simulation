#include <chrono>
#include <iostream>
#include <string>

#include "c_sim.hpp"
#include "devices.h"		// Four_Bit_Counter Device
#include "utils.h"

int main () {
	// Verbosity flags. Set verbose & monitor_on equal to true to display verbose simulation output in the console.
	bool monitor_on = false;
	
	int number_of_runs = 50;
	
	// Instantiate the top-level Device (the Simulation).
	Simulation* sim = new Simulation("test_sim");
	
	// Add a 4-bit counter device.
	sim->AddComponent(new N_Bit_Counter_AIO(sim, "test_counter", 4, monitor_on, {{"run", true}}));
	
	// Once we have added all our devices, call the simulation's Stabilise() method to finish setup.
	sim->Stabilise();
	
	// Add a Clock and connect it to the clk input on the counter.
	// The Clock output will be a repeating pattern of false, true, false, true, etc, starting on false on the first tick.
	sim->AddClock("clock_0", {false, true}, monitor_on);
	sim->ClockConnect("clock_0", "test_counter", "clk");
	
	// We need a pointer to the Device so we can directly interrogate it's out pins rather than using a Probe,
	// as a Probe will slow things down significantly.
	Component* counter = sim->GetChildComponentPointer("test_counter");
	
	Converter count_converter(4);
	
	std::vector<std::string> counter_output_names = {"q_0", "q_1", "q_2", "q_3"};
	std::vector<bool> read_states = {};
	std::vector<int> last_5_counts = {};
	std::vector<long long> run_times = {};
	
	for (int run_index = 0; run_index < number_of_runs; run_index ++) {
		std::cout << std::endl << "Run : " << run_index << std::endl;
		auto t1 = std::chrono::high_resolution_clock::now();
		sim->Run(49990, true, false, true);
		for (int last_5_index = 0; last_5_index < 5; last_5_index ++) {
			sim->Run(2, false, false, true);
			for (int pin_index = 0; pin_index < 4; pin_index ++) {
				read_states.emplace_back(counter->GetPinState(counter->GetPinPortIndex(counter_output_names[pin_index])));
			}
			last_5_counts.emplace_back(count_converter.StatesToInt(read_states));
			read_states.clear();
		}
		auto t2 = std::chrono::high_resolution_clock::now();
		long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
		run_times.emplace_back(microseconds);
		for (int last_5_index = 0; last_5_index < 5; last_5_index ++) {
			std::cout << last_5_counts[last_5_index] << std::endl;
		}
		last_5_counts.clear();
	}

	std::cout << std::endl;
	for (int index = 0; index < number_of_runs; index ++) {
		std::cout << run_times[index] << std::endl;
	}
	
	delete sim;
	
	return 0;
}

