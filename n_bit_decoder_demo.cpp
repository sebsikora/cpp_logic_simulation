#include <cmath>

#include "c_core.h"			// Core simulator functionality
#include "devices.h"		// N_Bit_Register Device

int main () {
	// Verbosity flags. Set verbose & monitor_on equal to true to display verbose simulation output in the console.
	bool verbose = false;
	bool monitor_on = false;
	bool print_probe_samples = true;
	
	// Set the select bus width of the decoder here.
	int select_bus_width = 3;
	
	// Instantiate the top-level Device (the Simulation).
	Simulation* sim = new Simulation("test_sim", 10, verbose);
	
	// Add the n-bit register Device. Note the parameterised decoder select bus width width.
	sim->AddComponent(new N_Bit_Decoder(sim, "test_decoder", select_bus_width, monitor_on));
	sim->ChildMakeProbable("test_decoder");
	
	// Once we have added all our devices, call the simulation's Stabilise() method to finish setup.
	sim->Stabilise();
	
	// Clock to trigger probes.
	sim->AddClock("clock_0", {false, true}, monitor_on);
	
	// Programmatically generate select input and output pins identifiers.
	std::vector<std::string> out_pins = {};
	std::vector<std::string> select_in_pins = {};
	for (int i = 0; i < select_bus_width; i ++) {
		std::string this_select_pin = "sel_" + std::to_string(i);
		select_in_pins.push_back(this_select_pin);
	}
	for (int i = 0; i < pow(2, select_bus_width); i ++) {
		std::string this_out_pin = "out_" + std::to_string(i);
		out_pins.push_back(this_out_pin);
	}
	
	// Add Probes connected to the clk, load and clear inputs of the register, to it's data input and output pins.
	sim->AddProbe("decoder_outputs", "test_sim:test_decoder", out_pins, "clock_0");
	sim->AddProbe("decoder_inputs", "test_sim:test_decoder", select_in_pins, "clock_0");
	
	// Cycle throught the select input combinations and trigger the Probes.
	bool first_tick = true;
	for (int selection = 0; selection < pow(2, select_bus_width); selection ++) {
		// For the current value of selection, set appropriate select input pins to '1'.
		for (int select_pin_index = 0; select_pin_index < select_bus_width; select_pin_index ++) {
			std::string select_pin_identifier = "sel_" + std::to_string(select_pin_index);
			// Does this position of the encoded selection value correspond to '1'?
			if ((selection & (1 << select_pin_index)) == (1 << select_pin_index)) {
				sim->ChildSet("test_decoder", select_pin_identifier, true);
			} else {
				sim->ChildSet("test_decoder", select_pin_identifier, false);
			}
		}
		if (first_tick) {
			sim->Run(1, true, verbose, false);
			first_tick = false;
		} else {
			sim->Run(1, false, verbose, false);
		}
	}
	sim->Run(1, false, verbose, print_probe_samples);
}


