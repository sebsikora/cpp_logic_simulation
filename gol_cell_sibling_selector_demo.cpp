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
	
	sim->AddComponent(new GameOfLife_Cell_SiblingSelector(sim, "test_selector", monitor_on, {{"not_clear", true}}));
	
	// Once we have added all our devices, call the simulation's Stabilise() method to finish setup.
	sim->Stabilise();
	
	// Add a Clock.
	// The Clock output will be a repeating pattern of false, true, false, true, etc, starting on false on the first tick.
	sim->AddClock("clock_0", {false, true}, monitor_on);
	sim->ClockConnect("clock_0", "test_selector", "clk");
	
	// Add Probes.
	sim->AddProbe("siblings_in", "test_sim:test_selector", {"sibling_0", "sibling_1", "sibling_2", "sibling_3", "sibling_4", "sibling_5", "sibling_6", "sibling_7"}, "clock_0");
	sim->AddProbe("selected_sibling", "test_sim:test_selector", {"selected_sibling"}, "clock_0");
	sim->AddProbe("update_flag", "test_sim:test_selector", {"update_flag"}, "clock_0");
	sim->AddProbe("sibling_counter", "test_sim:test_selector:sibling_counter", {"q_0", "q_1", "q_2", "q_3"}, "clock_0");
	
	sim->ChildSet("test_selector", "sibling_0", true);
	sim->ChildSet("test_selector", "sibling_1", false);
	sim->ChildSet("test_selector", "sibling_2", true);
	sim->ChildSet("test_selector", "sibling_3", false);
	sim->ChildSet("test_selector", "sibling_4", true);
	sim->ChildSet("test_selector", "sibling_5", false);
	sim->ChildSet("test_selector", "sibling_6", true);
	sim->ChildSet("test_selector", "sibling_7", false);
	
	sim->ChildSet("test_selector", "not_clear", false);
	sim->ChildSet("test_selector", "not_clear", true);
	
	sim->Run(20, true, verbose, print_probe_samples);

	delete sim;
	
	return 0;
}
