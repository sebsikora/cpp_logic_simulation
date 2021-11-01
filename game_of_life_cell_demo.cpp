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
	
	// Instantiate the top-level Device (the Simulation).
	Simulation* sim = new Simulation("test_sim", 20, verbose);
	
	sim->AddComponent(new GameOfLife_Cell(sim, "test_cell", monitor_on, {{"not_clear_cycle", true}, {"not_clear_state", true}, {"not_preset_state", true}}));
	
	// Once we have added all our devices, call the simulation's Stabilise() method to finish setup.
	sim->Stabilise();
	
	// Add a Clock.
	// The Clock output will be a repeating pattern of false, true, false, true, etc, starting on false on the first tick.
	sim->AddClock("clock_0", {false, true}, monitor_on);
	sim->ClockConnect("clock_0", "test_cell", "clk");
	
	// Add Probes.
	sim->AddProbe("counter_outputs", "test_sim:test_cell:counter", {"q_0", "q_1", "q_2", "q_3"}, "clock_0");
	sim->AddProbe("cell_siblings_out", "test_sim:test_cell", {"sibling_0_out", "sibling_0_out", "sibling_1_out", "sibling_2_out", "sibling_3_out", "sibling_4_out", "sibling_5_out", "sibling_6_out", "sibling_7_out"}, "clock_0");
	//~sim->AddProbe("selector_count", "test_sim:test_cell:selector:sibling_counter", {"q_0", "q_1", "q_2", "q_3"}, "clock_0");
	//~sim->AddProbe("selected_sibling", "test_sim:test_cell:selector", {"selected_sibling", "update_flag"}, "clock_0");
	//~sim->AddProbe("counter_run_in", "test_sim:test_cell:counter", {"run"}, "clock_0");
	
	// Possible sibling states.
	std::vector<std::vector<bool>> sibling_states = {};
	for (int i = 0; i < 9; i ++) {
		std::vector<bool> states = {};
		for (int j = 0; j < i; j ++) {
			states.push_back(true);
		}
		for (int j = i; j < 8; j ++) {
			states.push_back(false);
		}
		sibling_states.push_back(states);
	}

	// We will cycle through all the available sibling states, IE from all dead to all alive, for both cell starting states.
	
	// We will start with a 'dead' starting state.
	for (int sibling_count_index = 0; sibling_count_index < 9; sibling_count_index ++) {
		for (int k = 0; k < 8; k ++) {
			std::cout << sibling_states[sibling_count_index][k];
		}
		std::cout << std::endl;
		// Make sure Cell is 'dead' and reset.
		sim->ChildSet("test_cell", "not_clear_cycle", false);
		sim->ChildSet("test_cell", "not_clear_cycle", true);
		sim->ChildSet("test_cell", "not_clear_state", false);
		sim->ChildSet("test_cell", "not_clear_state", true);
		// Now set sibling inputs.
		for (int pin_index = 0; pin_index < 8; pin_index ++) {
			std::string sibling_input_name = "sibling_" + std::to_string(pin_index) + "_in";
			sim->ChildSet("test_cell", sibling_input_name, sibling_states[sibling_count_index][pin_index]);
		}
		// Now run for 18 ticks to integrate the sibling inputs and get the new Cell state.
		sim->Run(18, true, verbose, print_probe_samples, false);
	}
	
	// Now we sweep through all possible sibling states with an initial 'alive' state.
	for (int sibling_count_index = 0; sibling_count_index < 9; sibling_count_index ++) {
		for (int k = 0; k < 8; k ++) {
			std::cout << sibling_states[sibling_count_index][k];
		}
		std::cout << std::endl;
		// Make sure Cell is 'alive' and reset.
		sim->ChildSet("test_cell", "not_clear_cycle", false);
		sim->ChildSet("test_cell", "not_clear_cycle", true);
		sim->ChildSet("test_cell", "not_preset_state", false);
		sim->ChildSet("test_cell", "not_preset_state", true);
		// Now set sibling inputs.
		for (int pin_index = 0; pin_index < 8; pin_index ++) {
			std::string sibling_input_name = "sibling_" + std::to_string(pin_index) + "_in";
			sim->ChildSet("test_cell", sibling_input_name, sibling_states[sibling_count_index][pin_index]);
		}
		// Now run for 18 ticks to integrate the sibling inputs and get the new Cell state.
		sim->Run(18, true, verbose, print_probe_samples, false);
	}

	delete sim;
	
	return 0;
}
