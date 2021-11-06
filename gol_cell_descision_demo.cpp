#include <iostream>

#include "c_core.h"			// Core simulator functionality
#include "devices.h"
#include "game_of_life.h"
#include "utils.h"

int main () {
	// Verbosity flags. Set verbose & monitor_on equal to true to display verbose simulation output in the console.
	bool verbose = false;
	bool monitor_on = false;
	bool print_probe_samples = true;
	
	// Instantiate the top-level Device (the Simulation).
	Simulation* sim = new Simulation("test_sim", verbose);
	
	sim->AddComponent(new GameOfLife_Cell_Decider(sim, "test_decider", monitor_on));
	
	// Once we have added all our devices, call the simulation's Stabilise() method to finish setup.
	sim->Stabilise();
	
	// Add a Clock.
	// The Clock output will be a repeating pattern of false, true, false, true, etc, starting on false on the first tick.
	sim->AddClock("clock_0", {false, true}, monitor_on);
	
	// Add Probes.
	sim->AddProbe("decider_alive_in", "test_sim:test_decider", {"alive_in"}, "clock_0");
	sim->AddProbe("decider_count_in", "test_sim:test_decider", {"count_in_0", "count_in_1", "count_in_2"}, "clock_0");
	sim->AddProbe("decider_alive_out", "test_sim:test_decider", {"alive_out"}, "clock_0");
	
	std::vector<bool> prior_state = {false, true};
	for (const auto& this_state : prior_state) {
		sim->ChildSet("test_decider", "alive_in", this_state);
		for (int value = 0; value < 8; value ++) {
			std::vector<bool> counter_outputs = IntToStates(value, 3);
			for (int output_index = 0; output_index < 3; output_index ++) {
				std::string count_pin_name = "count_in_" + std::to_string(output_index);
				sim->ChildSet("test_decider", count_pin_name, counter_outputs[output_index]);
			}
			if ((value == 0) && (!this_state)) {
				sim->Run(1, true, verbose, false);
			} else if ((value == 7) && (this_state)) {
				sim->Run(1, false, verbose, print_probe_samples);
			} else {
				sim->Run(1, false, verbose, false);
			}
		}
	}
	
	delete sim;
	
	return 0;
}
