#include <iostream>

#include "c_core.h"			// Core simulator functionality
#include "devices.h"		// NxM_Bit_Mux Device

int main () {
	// Verbosity flags. Set verbose & monitor_on equal to true to display verbose simulation output in the console.
	bool verbose = false;
	bool monitor_on = true;
	bool print_probe_samples = true;
	
	int data_bus_width = 4;
	int data_bus_count = 3;
	
	// Instantiate the top-level Device (the Simulation).
	std::string sim_name = "test_sim";
	Simulation sim = Simulation(sim_name, 10, verbose);
	
	// Generate some names we will need to programmatically interact with the Device.
	std::string mux_name = "test_mux";
	std::string sim_mux_name = sim_name + ":" + mux_name;
	
	// Add the n-bit register Device. Note the parameterised register width.
	sim.AddComponent(new NxM_Bit_Mux(&sim, mux_name, data_bus_count, data_bus_width, monitor_on));
		
	// Once we have added all our devices, call the simulation's Stabilise() method to finish setup.
	sim.Stabilise();
	
	// Add a Clock and connect it to the clk input on the register.
	// The Clock output will be a repeating pattern of false, true, false, true, etc, starting on false on the first tick.
	sim.AddClock("clock_0", {false, true}, monitor_on);
	
	// Programmatically generate the required vectors of register data input and output pin names.
	// We will need these for attaching probes and driving the data input pins.
	std::vector<std::string> d_out_pins = {};
	for (int i = 0; i < data_bus_width; i ++) {
		std::string this_out_pin = "d_out_" + std::to_string(i);
		d_out_pins.push_back(this_out_pin);
	}
	std::vector<std::vector<std::string>> all_d_in_pins = {};
	for (int i = 0; i < data_bus_count; i ++) {
		std::vector<std::string> d_in_pins = {};
		for (int j = 0; j < data_bus_width; j ++) {
			std::string this_in_pin = "d_in_" + std::to_string(i) + "_" + std::to_string(j);
			d_in_pins.push_back(this_in_pin);
		}
		all_d_in_pins.push_back(d_in_pins);
	}
	
	// Add probes to mux output data bus and input data busses.
	sim.AddProbe("mux_data_out_bus", sim_mux_name, d_out_pins, "clock_0");
	for (int bus_index = 0; bus_index < data_bus_count; bus_index ++) {
		std::string mux_input_bus_probe_identifier = "mux_data_in_bus_" + std::to_string(bus_index);
		sim.AddProbe(mux_input_bus_probe_identifier, sim_mux_name, all_d_in_pins[bus_index], "clock_0");
	}
	sim.AddProbe("mux_select_in", sim_mux_name, {"sel_0", "sel_1"}, "clock_0");
	
	// Set data input busses to ascending values.
	for (int bus_index = 0; bus_index < data_bus_count; bus_index ++) {
		for (int pin_index = 0; pin_index < data_bus_width; pin_index ++) {
			if ((bus_index & (1 << pin_index)) == (1 << pin_index)) {
				sim.ChildSet(mux_name, all_d_in_pins[bus_index][pin_index], true);
			} else {
				sim.ChildSet(mux_name, all_d_in_pins[bus_index][pin_index], false);
			}
		}
	}
	sim.Run(1, true, false, false);
	sim.ChildSet(mux_name, "sel_0", true);
	sim.ChildSet(mux_name, "sel_1", false);
	sim.Run(1, false, false, false);
	sim.ChildSet(mux_name, "sel_0", false);
	sim.ChildSet(mux_name, "sel_1", true);
	sim.Run(1, false, false, false);
	sim.ChildSet(mux_name, "sel_0", true);
	sim.ChildSet(mux_name, "sel_1", true);
	sim.Run(1, false, false, true);
	
	return 0;
}


