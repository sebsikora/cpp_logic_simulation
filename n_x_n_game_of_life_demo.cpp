#include <vector>
#include <string>

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
	int cell_count = x_dimension * x_dimension;
	
	// Instantiate the top-level Device (the Simulation).
	Simulation* sim = new Simulation("test_sim", 20, verbose);
	
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
	sim->AddComponent(new GameOfLife(sim, "game_of_life", x_dimension, monitor_on, in_pin_default_states));
	sim->ChildMakeProbable("game_of_life");
	
	// Once we have added all our devices, call the simulation's Stabilise() method to finish setup.
	sim->Stabilise();
	
	// Add a Clock.
	// The Clock output will be a repeating pattern of false, true, false, true, etc, starting on false on the first tick.
	sim->AddClock("clock_0", {false, true}, monitor_on);
	sim->ClockConnect("clock_0", "game_of_life", "clk");
	
	sim->ChildSet("game_of_life", "not_clear_cycle", false);
	sim->ChildSet("game_of_life", "not_clear_cycle", true);
	for (int cell_index = 0; cell_index < cell_count; cell_index ++) {
		sim->ChildSet("game_of_life", not_clear_state_identifiers[cell_index], false);
		sim->ChildSet("game_of_life", not_clear_state_identifiers[cell_index], true);
	}
	sim->AddProbe("cell_states", "test_sim:game_of_life", {output_identifiers}, "clock_0", x_dimension);
	sim->ChildSet("game_of_life", not_preset_state_identifiers[31], false);
	sim->ChildSet("game_of_life", not_preset_state_identifiers[31], true);
	sim->ChildSet("game_of_life", not_preset_state_identifiers[40], false);
	sim->ChildSet("game_of_life", not_preset_state_identifiers[40], true);
	sim->ChildSet("game_of_life", not_preset_state_identifiers[49], false);
	sim->ChildSet("game_of_life", not_preset_state_identifiers[49], true);
	sim->Run(0, true, verbose, print_probe_samples, false);
	
	delete sim;
	
	return 0;
}
