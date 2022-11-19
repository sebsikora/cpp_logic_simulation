#include <unistd.h>					// sleep().
#include <cmath>					// pow().

#include "c_core.h"					// Core simulator functionality
#include "control_panel.h"		// SimpleTerminal MagicDevice

int main () {
	// Verbosity flags. Set verbose & monitor_on equal to true to display verbose simulation output in the console.
	bool verbose = false;
	bool monitor_on = false;
	
	int seconds_to_pause_simulation = 30;
	
	// Instantiate the top-level Device (the Simulation).
	Simulation sim("test_sim", verbose);
	
	// Add the ControlPanel MagicDevice and call stabilise.
	std::string panel_recipe("AP,panel_zero,2\nAP,panel_one,2\n");
	panel_recipe += "AD,0,0,test_0,16\nAD,0,1,test_2,16\n";
	panel_recipe += "AS,1,0,switch_0,false,false\nAS,0,0,switch_1,false,false\nAS,0,0,switch_2,true,true\n";
	panel_recipe += "AB,0,2,switch_bank_0,12,false,false\nAB,1,1,switch_bank_1,16,true,false\n";
	
	sim.AddComponent(new ControlPanel(&sim, "control_panel", panel_recipe, monitor_on));
	sim.Stabilise();
	
	// Pause the simulation.
	sleep(seconds_to_pause_simulation);
	
	return 0;
}

