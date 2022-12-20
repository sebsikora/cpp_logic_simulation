#include "c_sim.hpp"
#include "uart.hpp"		// JK flip-flop Device

int main () {
	// Instantiate the top-level Device (the Simulation).
	Simulation sim("test_sim");
	
	// Add a jk flip-flop Device.
	sim.AddComponent(new Uart(&sim, "test_uart", true));
	
	sim.Stabilise();

	// Add a Clock and connect it to the clk input on the jk flip-flop.
	// The Clock output will be a repeating pattern of false, true, false, true, etc, starting on false on the first tick.
	//~sim.AddClock("clock_0", {false, true}, true);
	//~sim.ClockConnect("clock_0", "test_ff", "clk");
	
	// Add two Probes and connect them to the jk flip-flop's outputs and clk input.
	// Note - we need to use the target Component's 'full name' to connect a Probe.
	//~sim.AddProbe("flip-flop outputs", "test_sim:test_ff", {"q", "not_q"}, "clock_0");
	//~sim.AddProbe("flip-flop clk input", "test_sim:test_ff", {"clk"}, "clock_0");

	//~// Run the simulation for ten ticks.
	sim.Run(0, true);
	
	return 0;
}
