#include <vector>
#include <string>
#include <chrono>

#include "c_core.h"			// Core simulator functionality
#include "devices.h"
#include "game_of_life.h"
#include "utils.h"

int main () {
	// Verbosity flags. Set verbose & monitor_on equal to true to display verbose simulation output in the console.
	bool verbose = false;
	bool monitor_on = false;
	bool print_probe_samples = true;
	
	// Square grid base dimension.
	int x_dimension = 9;
	// Number of game iterations to simulate.
	int iteration_count = 3;
	
	int cell_count = x_dimension * x_dimension;
	int tick_count = 18 * iteration_count;
	
	// Instantiate the top-level Device (the Simulation).
	Simulation sim("test_sim", verbose, {true, 1});
	
	std::vector<state_descriptor> in_pin_default_states = {{"not_clear_cycle", true}};
	std::vector<std::string> output_identifiers = {};
	std::vector<std::string> not_clear_state_identifiers = {};
	std::vector<std::string> not_preset_state_identifiers = {};
	
	for (int cell_id = 0; cell_id < cell_count; cell_id ++) {
		std::string not_clear_state_identifier = "cell_" + std::to_string(cell_id) + "_not_clear_state";
		std::string not_preset_state_identifier = "cell_" + std::to_string(cell_id) + "_not_preset_state";
		std::string output_identifier = "cell_" + std::to_string(cell_id) + "_state";
		in_pin_default_states.push_back({not_clear_state_identifier, true});
		in_pin_default_states.push_back({not_preset_state_identifier, true});
		not_clear_state_identifiers.push_back(not_clear_state_identifier);
		not_preset_state_identifiers.push_back(not_preset_state_identifier);
		output_identifiers.push_back(output_identifier);
	}
	sim.AddComponent(new GameOfLife(&sim, "game_of_life", x_dimension, monitor_on, in_pin_default_states));
	
	// Once we have added all our devices, call the simulation's Stabilise() method to finish setup.
	sim.Stabilise();
	
	// Add a Clock.
	// The Clock output will be a repeating pattern of false, true, false, true, etc, starting on false on the first tick.
	sim.AddClock("clock_0", {false, true}, monitor_on);
	sim.ClockConnect("clock_0", "game_of_life", "clk");
	
	sim.AddProbe("cell_states", "test_sim:game_of_life", {output_identifiers}, "clock_0", {18, x_dimension, {" ", "■"}});
	
	sim.ChildSet("game_of_life", "not_clear_cycle", false);
	sim.ChildSet("game_of_life", "not_clear_cycle", true);

	// If you uncomment the below the array of cells will be cleared at the start and then three in a row
	// will be set 'alive'. This will create the simplest 'blinker' pattern.
	for (int cell_index = 0; cell_index < cell_count; cell_index ++) {
		sim.ChildSet("game_of_life", not_clear_state_identifiers[cell_index], false);
		sim.ChildSet("game_of_life", not_clear_state_identifiers[cell_index], true);
	}
	
	int initial_offset = (int)(cell_count / 2);
	sim.ChildSet("game_of_life", not_preset_state_identifiers[initial_offset - x_dimension], false);
	sim.ChildSet("game_of_life", not_preset_state_identifiers[initial_offset - x_dimension], true);
	sim.ChildSet("game_of_life", not_preset_state_identifiers[initial_offset], false);
	sim.ChildSet("game_of_life", not_preset_state_identifiers[initial_offset], true);
	sim.ChildSet("game_of_life", not_preset_state_identifiers[initial_offset + x_dimension], false);
	sim.ChildSet("game_of_life", not_preset_state_identifiers[initial_offset + x_dimension], true);
	// ---------------------------------------------------------------------------------------------------
	
	auto t1 = std::chrono::high_resolution_clock::now();
	sim.Run(tick_count, true, verbose, false, true);
	auto t2 = std::chrono::high_resolution_clock::now();
	sim.Run(1, false, verbose, print_probe_samples, false);
	
	long long us_int = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
	std::cout << us_int << std::endl;
	
	return 0;
}
