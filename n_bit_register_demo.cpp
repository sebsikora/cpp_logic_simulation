#include "c_core.h"			// Core simulator functionality
#include "devices.h"		// N_Bit_Register Device

int main () {
	// Verbosity flags. Set verbose & monitor_on equal to true to display verbose simulation output in the console.
	bool verbose = false;
	bool monitor_on = false;
	bool print_probe_samples = true;
	
	// Set the desired bit-width of the register here.
	int register_width = 4;
	
	// Instantiate the top-level Device (the Simulation).
	Simulation* sim = new Simulation("test_sim", 10, verbose);
	
	// Add the n-bit register Device. Note the parameterised register width.
	sim->AddComponent(new N_Bit_Register(sim, "test_register", register_width, monitor_on, {{"clr", true}, {"load", false}}));
	
	// Once we have added all our devices, call the simulation's Stabilise() method to finish setup.
	sim->Stabilise();
	
	// Add a Clock and connect it to the clk input on the register.
	// The Clock output will be a repeating pattern of false, true, false, true, etc, starting on false on the first tick.
	sim->AddClock("clock_0", {false, true}, monitor_on);
	sim->ClockConnect("clock_0", "test_register", "clk");
	
	// Programmatically generate the required vectors of register data input and output pin names.
	// We will need these for attaching probes and driving the data input pins.
	std::vector<std::string> d_out_pins = {};
	std::vector<std::string> d_in_pins = {};
	for (int i = 0; i < register_width; i ++) {
		std::string this_out_pin = "d_out_" + std::to_string(i);
		std::string this_in_pin = "d_in_" + std::to_string(i);
		d_out_pins.push_back(this_out_pin);
		d_in_pins.push_back(this_in_pin);
	}

	sim->ChildMakeProbable("test_register");
	
	// Add Probes connected to the clk, load and clear inputs of the register, to it's data input and output pins.
	sim->AddProbe("reg_data_out", "test_sim:test_register", d_out_pins, "clock_0");
	sim->AddProbe("reg_data_in", "test_sim:test_register", d_in_pins, "clock_0");
	sim->AddProbe("reg_clk_in", "test_sim:test_register", {"clk"}, "clock_0");
	sim->AddProbe("reg_load_in", "test_sim:test_register", {"load"}, "clock_0");
	sim->AddProbe("reg_clr_in", "test_sim:test_register", {"clr"}, "clock_0");
	
	//~// Set input control pins to 'clear'.
	//~sim->ChildSet("test_register", "load", false);
	//~sim->ChildSet("test_register", "clr", true);
	// Run the sim for 3 ticks to generate a high to low clock transition and clear the register.
	sim->Run(3, true, verbose);
	
	// Set input control pins to 'load'.
	sim->ChildSet("test_register", "load", true);
	sim->ChildSet("test_register", "clr", false);
	// Set all the register data input pins to high (true).
	for (const auto& in_pin: d_in_pins) {
		sim->ChildSet("test_register", in_pin, true);
	}
	// Run the sim for 2 ticks to latch the input data.
	sim->Run(2, false, verbose);
	

	// Set the odd-numbered register data input pins to high (true) and the even ones to low (false).
	int i = 0;
	for (const auto& in_pin: d_in_pins) {
		bool pin_state = false;
		if (i % 2 > 0) {
			pin_state = true;
		} else {
			pin_state = false;
		}
		sim->ChildSet("test_register", in_pin, pin_state);
		i ++;
	}
	// Run the sim for 2 ticks to latch the input data.
	sim->Run(2, false, verbose);
	
	// Set input control pins to 'clear'.
	sim->ChildSet("test_register", "load", false);
	sim->ChildSet("test_register", "clr", true);
	sim->Run(2, false, verbose, print_probe_samples);
	
	delete sim;
	
	return 0;
}


