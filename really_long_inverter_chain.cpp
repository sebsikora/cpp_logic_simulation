#include <iostream>
#include <chrono>

#include "c_core.h"					// Core simulator functionality
#include "devices.h"				// Multiple Really Long Inverter Chain.

int main () {
	// Verbosity flags. Set verbose & monitor_on equal to true to display verbose simulation output in the console.
	bool verbose = false;
	bool monitor_on = false;
	
	// Instantiate the top-level Device (the Simulation).
	Simulation* sim = new Simulation("test_sim", verbose, {true, 1}, 100000);
	
	sim->AddComponent(new MRLIC(sim, "mrlic", monitor_on, {{"in", false}}));
	
	// Once we have added all our devices, call the simulation's Stabilise() method to finish setup.
	sim->Stabilise();

	auto t1 = std::chrono::high_resolution_clock::now();
	bool setting = true;
	for (int i = 0; i < 5000; i ++) {
		sim->ChildSet("mrlic", "in", setting);
		setting = !setting;
	}
	auto t2 = std::chrono::high_resolution_clock::now();
	long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
	std::cout << microseconds << std::endl;
	
	delete sim;
	
	return 0;
}

