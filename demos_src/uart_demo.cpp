#include "c_sim.hpp"
#include "uart.hpp"
#include "devices.h"
#include "c_gates.hpp"

int main () {
	// Instantiate the top-level Device (the Simulation).
	Simulation sim("test_sim");
	
	// Add the UART, and an inverter and jk flip-flop which we will use to delay the write
	// signal by 1 clock cycle.
	sim.AddComponent(new Uart(&sim, "test_uart", true));
	sim.AddComponent(new Inverter(&sim, "not", false));
	sim.AddComponent(new JK_FF_ASPC(&sim, "jk_ff", false, {{"not_c", true}}));

	sim.ChildConnect("test_uart", {"d_out_0", "test_uart", "d_in_0"});
	sim.ChildConnect("test_uart", {"d_out_1", "test_uart", "d_in_1"});
	sim.ChildConnect("test_uart", {"d_out_2", "test_uart", "d_in_2"});
	sim.ChildConnect("test_uart", {"d_out_3", "test_uart", "d_in_3"});
	sim.ChildConnect("test_uart", {"d_out_4", "test_uart", "d_in_4"});
	sim.ChildConnect("test_uart", {"d_out_5", "test_uart", "d_in_5"});
	sim.ChildConnect("test_uart", {"d_out_6", "test_uart", "d_in_6"});
	sim.ChildConnect("test_uart", {"d_out_7", "test_uart", "d_in_7"});

	sim.ChildConnect("test_uart", {"data_ready", "not"});
	sim.ChildConnect("test_uart", {"data_ready", "jk_ff", "j"});
	sim.ChildConnect("test_uart", {"data_ready", "test_uart", "read"});
	sim.ChildConnect("not", {"jk_ff", "k"});
	sim.ChildConnect("jk_ff", {"q", "test_uart", "write"});

	sim.Connect("true", "jk_ff", "not_p");
	
	sim.Stabilise();

	sim.AddClock("clock_0", {false, true}, false);
	sim.ClockConnect("clock_0", "test_uart", "clk");
	sim.ClockConnect("clock_0", "jk_ff", "clk");

	sim.ChildSet("jk_ff", "not_c", false);
	sim.ChildSet("jk_ff", "not_c", true);

	sim.Run(50000000, true);
	
	return 0;
}
