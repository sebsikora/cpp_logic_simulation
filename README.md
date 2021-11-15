# cpp_logic_simulation

© 2021 Dr Sebastien Sikora.

[seb.nf.sikora@protonmail.com](mailto:seb.nf.sikora@protonmail.com)

Updated 15/11/2021.

What is it?
-------------------------

I created cpp_logic_simulation to explore C++ language features and development methodologies that I do not usually use in my C++ work - due to the resource-constrained nature of the typical target platforms (eg, 512 ~ 2K bytes RAM) - and to scratch a personal itch, having felt inspired by the idea of creating some sort of logic simulator for a number of years.

cpp_logic_simulation is a framework for constructing simulations of digital [logic circuits](https://learn.sparkfun.com/tutorials/digital-logic/all). The logic circuits comprise instances of a *Component* class, sub-classed into *Gate* and *Device* classes. *Gates* are n-input representations of the [logical operators](https://learn.sparkfun.com/tutorials/digital-logic/all#combinational-logic) NOT, AND, OR, NAND, NOR and XOR. Arbitrary numbers of *Gates* can be interconnected within *Devices*. These *Devices* can then interconnected nested-within further *Devices*, ad-nauseum, and in this way arbitrarily complex logic circuits can be assembled from the primary *Gate* building blocks. [See](./sim_src/devices/devices.cpp) `./sim_src/devices/devices.cpp` for examples.

A further sub-class of the *Device* class is the *Simulation* class, which lies at the top-level and contains the *Device* or *Devices* that comprise the logic circuit. The *Simulation* can also contain members of two additional utility classes, *Clocks* to drive the contained logic circuit, and *Probes*, which can sample and display the logical state of any inputs or outputs of *Gates* or *Devices* within the circuit. The state of the logic circuit is recalculated automatically in response to any state changes at it's inputs. However, sampling and displaying state using *Probes* can only occur while the simulation is 'running', with the circuit driven via one or more *Clocks*.

To do anything more interesting than view *Probe* output tables of changing logic levels within the circuit, a way to 'break the fourth wall' and interface with system resources 'outside' of the simulation is required. This is provided by a final *Device* sub-class, the *MagicDevice*. In addition to the usual *Device* functionality, *MagicDevices* contain special custom code (the *MagicEngine*) to interact with system resources outside of the simulation, and interfaces that hook into the simulated operation of the logic-circuit, and vice-versa. This allows us to create *MagicDevices* that, for example, behave as a RAM IC by accessing data contained in an array, a ROM IC by accessing data contained in a text file, or even a UART-like IC communicating with a remote text terminal! See `./sim_src/magic_devices/simple_ram.cpp`, `simple_rom.cpp` and `simple_terminal.cpp` for examples.

A basic example.
-------------------------

Let's dive-in and make something.

[Sequential](https://en.wikipedia.org/wiki/Sequential_logic) digital logic circuits need a way to store state, and to this they typically make use of [flip-flops](https://en.wikipedia.org/wiki/Flip-flop_(electronics)), of which there are a number of types. The most versatile type is the [JK flip-flop](https://www.electronics-tutorials.ws/sequential/seq_2.html), known as a 'universal' flip-flop as it can be configured to behave as any other kind of flip-flop.

Let's make a master-slave JK flip-flop, as they are completely insensitive to the duration of input signals (*edge-triggered*) and as-such are an ideal building-block for use in completely static sequential circuits.

```cpp
// ms_jk_ff.cpp

#include "c_core.h" 	// Include core simulation functionality

int main () {
	bool verbose = false;
	
	Simulation sim("test_sim", verbose);	// Instantiate top-level Simulation Device.
						// verbose flag turns off 'verbose output' for now.
	
	// Master-slave JK flip-flop can be assembled from 8 NAND gates, two three-input and six two-input,
	// and a single inverter (NOT gate) as shown here:
	//
	// https://www.electronics-tutorials.ws/sequential/seq_2.html
	//
	// Simulation's member function
	// AddGate(std::string const& component_name,                - Unique identifier string.
	//         std::string const& component_type,                - EG and, or, nand, nor, xor, not.
	//         std::vector<std::string> const& in_pin_names,     - Number of inputs determined from names.
	//         bool monitor_on);                                 - If flag = true changes in input or output
	//					                       states are reported on the console.
	sim.AddGate("nand_1", "nand", {"input_0", "input_1", "input_2"}, false);
	sim.AddGate("nand_2", "nand", {"input_0", "input_1", "input_2"});
	sim.AddGate("nand_3", "nand", {"input_0", "input_1"});
	sim.AddGate("nand_4", "nand", {"input_0", "input_1"});
	sim.AddGate("nand_5", "nand", {"input_0", "input_1"});
	sim.AddGate("nand_6", "nand", {"input_0", "input_1"});
	sim.AddGate("nand_7", "nand", {"input_0", "input_1"});
	sim.AddGate("nand_8", "nand", {"input_0", "input_1"});
	sim.AddGate("not_1", "not");	// For not Gates we can leave off the in pins vector,
					// it will be replaced by a single "input".
	
	// Add a Clock.
	//
	// Simulation's member function
	// AddClock(std::string const& clock_name,                   - Unique identifier string.
	//          std::vector<bool> const& toggle_pattern,         - Sequence of false/true values through which the Clock will step.
	//          bool monitor_on);                                - If flag = true changes in input or output states are reported on the console.
	//
	sim.AddClock("clock_0", {false, true}, false);
	
	sim.ClockConnect("clock_0", "nand_1", "input_2");	// Connect the clock where needed.
	sim.ClockConnect("clock_0", "nand_2", "input_2");	// For master-slave JK flip-flop clock connects to both master 
	sim.ClockConnect("clock_0", "not_1", "input");		// input NAND gates and NOT gate that feeds slave input NAND gates.
	// Interconnect components.
	//
	// Parent Device's member function
	// ChildConnect(std::string const& target_child_component_name,  - The name of the Component from which we wish to form a connection.
	//              std::vector<std::string> const& connection_parameters);
	//
	//            - Typically three connection parameters,
	//              std::string origin_pin_name       - The out pin name from which we wish to form a connection.
	//              std::string target_component_name - The name of the sibling component to which we wish to connect, use "parent"
	//                                                  to connect to an out pin of the parent Device.
	//              std::string target_pin_name       - The sibling Component in pin or parent Device out pin to which we wish to connect.
	//
	//            - Gates can omit the first connection parameter as they only have a single
	//              out pin "output".
	//
	sim.ChildConnect("nand_1", {"nand_3", "input_0"});
	sim.ChildConnect("nand_2", {"nand_4", "input_0"});
	sim.ChildConnect("nand_3", {"nand_4", "input_1"});
	sim.ChildConnect("nand_4", {"nand_3", "input_1"});
	
	sim.ChildConnect("nand_3", {"nand_5", "input_0"});
	sim.ChildConnect("nand_4", {"nand_6", "input_0"});

	sim.ChildConnect("not_1", {"nand_5", "input_1"});
	sim.ChildConnect("not_1", {"nand_6", "input_1"});
	
	sim.ChildConnect("nand_5", {"nand_7", "input_0"});
	sim.ChildConnect("nand_6", {"nand_8", "input_0"});
	sim.ChildConnect("nand_7", {"nand_8", "input_1"});
	sim.ChildConnect("nand_8", {"nand_7", "input_1"});
	
	sim.ChildConnect("nand_7", {"nand_2", "input_1"});
	sim.ChildConnect("nand_8", {"nand_1", "input_1"});

	// Tie the 'j' and 'k' in pins to the Simulation's 'true' utility pin.
	//
	// Simulation's member function
	// Connect(std::string const& origin_pin_name,           - The out pin name from which we wish to form a connection.
	//         std::string const& target_component_name,     - The unique identifier of the sibling component to which we wish to connect.
	//         std::string const& target_pin_name);          - The sibling Component in pin or parent Device out pin to which we wish to connect.
	//
	sim.Connect("true", "nand_1", "input_0");
	sim.Connect("true", "nand_2", "input_0");
	
	// If we want to probe any Components, we need to add them to the Simulation's probable Devices list.
	// We refer to them via their unique identifier string.
	sim.ChildMakeProbable("nand_7");
	sim.ChildMakeProbable("nand_8");
	
	// Once we have added all our devices, call the simulation's Stabilise()
	// method to finish setup.
	sim.Stabilise();
	
	// Simulation's member function
	// AddProbe(std::string const& probe_name,                       - The out pin name from which we wish to form a connection.
	//          std::string const& target_component_full_name,       - The unique identifier of the sibling component to which we wish to connect.
	//          std::vector<std::string> const& target_pin_names,    - The sibling Component in pin or parent Device out pin to which we wish to connect.
	//          std::string const& trigger_clock_name,               - ...
	//          probe_configuration probe_conf);                     - ...
	//
	sim.AddProbe("q", "test_sim:nand_7", {"output"}, "clock_0");
	sim.AddProbe("not_q", "test_sim:nand_8", {"output"}, "clock_0");
	
	// Run the simulation for 8 ticks. We should see the two probed out pins opposite
	// and toggling every other tick on the true->false clock transition.
	bool print_probe_samples = true;
	// Simulation's member function
	// Run(int number_of_ticks,                 - ...
	//     bool restart_flag,                   - ...
	//     bool verbose_output_flag,            - ...
	//     bool print_probes_flag,              - ...
	//     bool force_no_messages);             - ...
	//
	sim.Run(8, true, verbose, print_probe_samples);
		
	return 0;
}
```
Compile and run:
```
user@home:~/cpp_logic_simulation$ g++ -pthread -Wall -g -O3 -I sim_src/core/ -I sim_src/utils/ -I void_thread_pool/ sim_src/core/c_gate.cpp sim_src/core/c_m_engine.cpp sim_src/core/c_probe.cpp sim_src/core/c_sim.cpp sim_src/core/c_clock.cpp sim_src/core/c_comp.cpp sim_src/core/c_device.cpp sim_src/utils/utils.cpp sim_src/utils/strnatcmp.cpp void_thread_pool/void_thread_pool.cpp ms_jk_ff.cpp -o ms_jk_ff
user@home:~/cpp_logic_simulation$ ./ms_jk_ff

---------------------------- Simulation build started.  ----------------------------

(Simulation verbose output is off)

--------------------------- Simulation build completed.  ---------------------------

----------------------------- Simulation started (8).  -----------------------------

(Simulation verbose output is off)

-------------------------------------- Done.  --------------------------------------

---------------------------------- Probed values. ----------------------------------

Probe: q - test_sim:nand_7
T: 0   T
T: 1   T
T: 2   F
T: 3   F
T: 4   T
T: 5   T
T: 6   F
T: 7   F

Probe: not_q - test_sim:nand_8
T: 0   F
T: 1   F
T: 2   T
T: 3   T
T: 4   F
T: 5   F
T: 6   T
T: 7   T

-------------------------------------- Done.  --------------------------------------

user@home:~/cpp_logic_simulation$
```

Demos.
-------------------------

A number of demonstrations using the framework to simulate simple logic circuits are provided here. To run the simulations with maximally verbose output, detailing all of state changes within the circuit at runtime, set the `verbose` and `monitor_on` flags towards the top of the demos to `true`.
* `jk_ff_demo.cpp` - A master-slave JK flip-flop - [more](./sim_doc/jk_ff_demo.md)
* `4_bit_counter_demo.cpp` - A simple 4-bit counter - [more](./sim_doc/4_bit_counter_demo.md)
* `1_bit_register_demo.cpp` - A 1-bit storage register - [more](./sim_doc/1_bit_register_demo.md)
* `n_bit_register_demo.cpp` - A 4-bit register built programmatically from an array of 1-bit registers - [more](./sim_doc/n_bit_register_demo.md)
* `n_x_1_bit_mux_demo.cpp` - An N by 1-bit multiplexor built programmatically
* `n_bit_decoder_demo.cpp` - An N-bit decoder built programmatically
* `n_x_m_bit_mux_demo.cpp` - An N by M-bit multiplexor built programmatically from N by 1-bit muxes and an N-bit decoder

A significantly more complex demonstration is also provided here.
* `n_x_n_game_of_life_demo.cpp` - An N by N implementation of Conway's [Game of Life](https://en.wikipedia.org/wiki/Conway%27s_Game_of_Life) with a toroidal boundary condition at the edges.

Some *MagicDevice* demonstrations are provided here.
* `simple_rom_demo.cpp` - A ROM IC that pulls data from a text file - [more](./sim_doc/simple_rom_demo.md)
* `simple_ram_demo.cpp` - A RAM IC that stores data in an array - [more](./sim_doc/simple_ram_demo.md)
* `simple_terminal_demo.cpp` - A simple UART-like IC and accompanying terminal client - [more](./sim_doc/simple_terminal_demo.md)

Running the demos:
-------------------------

cpp_logic_simulation currently requires POSIX support for console io, and as-such cannot be compiled and run natively on Windows. It is possible to compile and run cpp_logic_simulation [on Windows](https://www.staff.ncl.ac.uk/andrey.mokhov/EEE1008/first-c-program.html) using [Cygwin](https://www.cygwin.com/), 'a large collection of GNU and Open Source tools which provide functionality similar to a GNU/Linux distribution on Windows', but that is outside the scope of this README. 

An elementary makefile is provided for the demos, so to compile a particular demo on GNU/Linux navigate to the root project directory on the command line and enter `make demo_name`, where `demo_name` is the name of one of the demo .cpp files with the .cpp file extension omitted.

For example, to compile and run the jk flip-flop demo ([jk_ff_demo.cpp](jk_ff_demo.cpp)), enter:

```
user@home:~/cpp_logic_simulation$ make jk_ff_demo
user@home:~/cpp_logic_simulation$ ./jk_ff_demo
```

If you would like to experiment with creating new *Devices* using the framework, when compiling make sure to include the `-pthread` compiler flag to add multi-threading support via the pthreads library.

Files:
-------------------------
* `./sim_src/core/` comprises the class prototypes and definitions that comprise the 'core engine'
* `./sim_src/devices/devices.cpp` contains example *Devices* constructed using the framework
* `./sim_src/devices/game_of_life/` contains a number of *Devices* that can together implement Conway's Game of Life.
* `./sim_src/magic_devices/` contains example *MagicDevices* constructed using the framework
* `./sim_src/simple_terminal_client/` contains the simple terminal client to accompany `simple_terminal_demo.cpp`
* `./sim_src/utils/` contains shared helper functions & macros for adding escape codes for colour terminal text
* `data.txt` is the data text file to accompany `simple_rom_demo.cpp`

Thoughts & limitations:
-------------------------

* Only logic circuits that are statically-stable state can be simulated.

License:
-------------------------

![LGPLv3 logo](sim_doc/220px-LGPLv3_Logo.png)

cpp_logic_simulation is distributed under the terms of the GNU Lesser General Public License v3.

cpp_logic_simulation is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

cpp_logic_simulation is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with cpp_logic_simulation.  If not, see <https://www.gnu.org/licenses/>.
