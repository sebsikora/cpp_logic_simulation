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
	Simulation sim = Simulation("test_sim", 20, verbose);
	
	sim.AddComponent(new GameOfLife_Cell(&sim, "test_cell", monitor_on, {{"not_clear_cycle", true}, {"not_clear_state", true}, {"not_preset_state", true}}));
	
	// Once we have added all our devices, call the simulation's Stabilise() method to finish setup.
	sim.Stabilise();

	sim.ChildPrintPinStates("test_cell", 4);
	
	// Add a Clock.
	// The Clock output will be a repeating pattern of false, true, false, true, etc, starting on false on the first tick.
	sim.AddClock("clock_0", {false, true}, monitor_on);
	sim.ClockConnect("clock_0", "test_cell", "clk");
	
	// Add Probes.
	//~sim.AddProbe("decider_alive_in", "test_sim:test_decider", {"alive_in"}, "clock_0");
	//~sim.AddProbe("decider_count_in", "test_sim:test_decider", {"count_in_0", "count_in_1", "count_in_2"}, "clock_0");
	//~sim.AddProbe("decider_alive_out", "test_sim:test_decider", {"alive_out"}, "clock_0");
	
	
	return 0;
}
