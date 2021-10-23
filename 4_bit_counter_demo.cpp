#include <iostream>

#include "c_core.h"					// Core simulator functionality
#include "devices.h"				// Four_Bit_Counter Device

int main () {
	// Verbosity flags. Set verbose & monitor_on equal to true to display verbose simulation output in the console.
	bool verbose = false;
	bool monitor_on = false;
	bool print_probe_samples = true;
	
	// Instantiate the top-level Device (the Simulation).
	Simulation* sim = new Simulation("test_sim", 10, verbose);
	
	// Add a 4-bit counter device.
	sim->AddComponent(new Four_Bit_Counter(sim, "test_counter", monitor_on, {{"run", true}}));
	
	// Add a Clock and connect it to the clk input on the counter.
	// The Clock output will be a repeating pattern of false, true, false, true, etc, starting on false on the first tick.
	sim->AddClock("clock_0", {false, true}, monitor_on);
	sim->ClockConnect("clock_0", "test_counter", "clk");
	
	// Once we have added all our devices, call the simulation's Stabilise() method to finish setup.
	sim->Stabilise();
	std::cout << " --- Probable Components --- " << std::endl;
	for (const auto& this_pointer : sim->m_probable_components) {
		std::cout << "Probable component : " << this_pointer->GetFullName() << std::endl;
	}
	std::cout << " --------------------------- " << std::endl;
	
	//~Component* component_pointer = sim->SearchForComponentPointer("test_sim:test_counter:jk_ff_0");
	//~component_pointer->PurgeComponent();
	//~delete component_pointer;
	
	//~std::cout << " --- Probable Components --- " << std::endl;
	//~for (const auto& this_pointer : sim->m_probable_components) {
		//~std::cout << "Probable component : " << this_pointer->GetFullName() << std::endl;
	//~}
	//~std::cout << " --------------------------- " << std::endl;
	
	sim->PurgeChildClock("clock_0");
	
	sim->PurgeComponent();
	delete sim;
	
	//~// Add two Probes and connect them to the counter's outputs and clk input.
	//~sim->AddProbe("clk_input", "test_sim:test_counter", {"clk"}, "clock_0");
	//~sim->AddProbe("counter_outputs", "test_sim:test_counter", {"q_0", "q_1", "q_2", "q_3"}, "clock_0");
	
	//~// Set the counter's run input to high (true).
	//~//sim.ChildSet("test_counter", "run", true);
	
	//~// Run the simulation for 33 ticks.
	//~sim->Run(33, true, verbose, print_probe_samples);
	
	return 0;
}

