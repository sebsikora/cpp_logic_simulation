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
	
	sim.AddComponent(new GameOfLife_Cell_SiblingSelector(&sim, "test_selector", monitor_on, {{"not_clear", true}}));
	
	// Once we have added all our devices, call the simulation's Stabilise() method to finish setup.
	sim.Stabilise();
	
	// Add a Clock.
	// The Clock output will be a repeating pattern of false, true, false, true, etc, starting on false on the first tick.
	sim.AddClock("clock_0", {false, true}, monitor_on);
	sim.ClockConnect("clock_0", "test_selector", "clk");
	
	// Add Probes.
	sim.AddProbe("siblings_in", "test_sim:test_selector", {"sibling_0", "sibling_1", "sibling_2", "sibling_3", "sibling_4", "sibling_5", "sibling_6", "sibling_7"}, "clock_0");
	sim.AddProbe("selected_sibling", "test_sim:test_selector", {"selcted_sibling"}, "clock_0");
	//~sim.AddProbe("decoder_out", "test_sim:test_selector:sibling_counter_decoder", {"out_0", "out_1", "out_2", "out_3", "out_4", "out_5", "out_6", "out_7"}, "clock_0");
	//~sim.AddProbe("counter_out", "test_sim:test_selector:sibling_counter", {"q_0", "q_1", "q_2"}, "clock_0");
	//~sim.AddProbe("counter_not_clear", "test_sim:test_selector:sibling_counter", {"not_clear"}, "clock_0");
	//~sim.AddProbe("counter_run", "test_sim:test_selector:sibling_counter", {"run"}, "clock_0");
	//~sim.AddProbe("counter_clk", "test_sim:test_selector:sibling_counter", {"clk"}, "clock_0");
	
	sim.ChildSet("test_selector", "sibling_0", true);
	sim.ChildSet("test_selector", "sibling_1", false);
	sim.ChildSet("test_selector", "sibling_2", true);
	sim.ChildSet("test_selector", "sibling_3", false);
	sim.ChildSet("test_selector", "sibling_4", true);
	sim.ChildSet("test_selector", "sibling_5", false);
	sim.ChildSet("test_selector", "sibling_6", true);
	sim.ChildSet("test_selector", "sibling_7", false);
	
	sim.Run(1, true, verbose, false);
	
	sim.ChildSet("test_selector", "not_clear", false);
	sim.ChildSet("test_selector", "not_clear", true);
	
	sim.Run(16, false, verbose, print_probe_samples);
	
	
	return 0;
}
