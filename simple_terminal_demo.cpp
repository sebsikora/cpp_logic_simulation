#include <unistd.h>					// sleep().
#include <cmath>					// pow().

#include "c_core.h"					// Core simulator functionality
#include "simple_terminal.h"		// SimpleTerminal MagicDevice

int main () {
	// Verbosity flags. Set verbose & monitor_on equal to true to display verbose simulation output in the console.
	bool verbose = false;
	bool monitor_on = false;
	bool print_probe_samples = true;
	
	// Demo parameters. A long message_to_send will result in long Probe sample printouts...
	int characters_to_clock_in = 10;
	std::string message_to_send = "Hello World!\nThis message clocked-out of the simulation!\n\nEnter upto " + std::to_string(characters_to_clock_in) + " characters here and in a few seconds they will be clocked-in to the simulation...\n\n";
	int seconds_to_pause_simulation = 10;
	
	// Instantiate the top-level Device (the Simulation).
	Simulation sim = Simulation("test_sim", 10, verbose);
	
	// Add the SimpleTerminal MagicDevice and call stabilise.
	sim.AddComponent(new SimpleTerminal(&sim, "test_terminal", monitor_on, {{"read", false}, {"write", true}}));
	sim.Stabilise();
	
	// Add a Clock and connect it to the clk input on the register.
	// The Clock output will be a repeating pattern of false, true, false, true, etc, starting on false on the first tick.
	sim.AddClock("clock_0", {false, true}, monitor_on);
	sim.ClockConnect("clock_0", "test_terminal", "clk");
	
	// Add Probes connected to the terminal's *read*, *write* & *clk* control input pins and *data_waiting* and data output bus pins.
	sim.AddProbe("terminal_data_out", "test_sim:test_terminal", {"d_out_0", "d_out_1", "d_out_2", "d_out_3", "d_out_4", "d_out_5", "d_out_6", "d_out_7"}, "clock_0");
	sim.AddProbe("terminal_read_in", "test_sim:test_terminal", {"read"}, "clock_0");
	sim.AddProbe("terminal_write_in", "test_sim:test_terminal", {"write"}, "clock_0");
	sim.AddProbe("terminal_data_waiting_out", "test_sim:test_terminal", {"data_waiting"}, "clock_0");
	sim.AddProbe("terminal_clk_in", "test_sim:test_terminal", {"clk"}, "clock_0");
	
	// Set the terminal's *read* control input to low (false), and *write* control input to high (true).
	sim.ChildSet("test_terminal", "read", false);
	sim.ChildSet("test_terminal", "write", true);
	
	// Run the simulation for a single tick.
	sim.Run(1, true);
	
	// Clock the message_to_send out of the terminal to the client one character at a time.
	for (const auto& character: message_to_send) {
		int ascii_value = int(character);
		int running_total = 0;
		// For the current character loop through the terminals data input pins and set appropriate states for the ASCII value.
		for (int i = 7; i > -1; i --) {
			std::string pin_name = "d_in_" + std::to_string(i);
			int position_value = pow(2, i);
			if (((ascii_value - running_total) - position_value) >= 0) {
				// This bit position should be a '1'...
				sim.ChildSet("test_terminal", pin_name, true);
				running_total += position_value;
			} else {
				sim.ChildSet("test_terminal", pin_name, false);
			}
		}
		// Run the simulation for two ticks to generate a high to low clock transition and output the character to the client.
		sim.Run(2, false);
	}
	
	// Pause the simulation.
	sleep(seconds_to_pause_simulation);
	
	// Set the terminal's *read* control input to high, and *write* control input to low.
	sim.ChildSet("test_terminal", "read", true);
	sim.ChildSet("test_terminal", "write", false);
	
	// Run the simulation for two ticks per character that we want to clock out of the terminals receive buffer.
	sim.Run(characters_to_clock_in * 2, false, verbose, print_probe_samples);
	
	// Need to call ShutDown() to cleanly kill the SimpleTerminal terminal_client process.
	sim.ShutDown();
	
	return 0;
}

