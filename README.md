# cpp_logic_simulation

© 2021 Dr Sebastien Sikora.

[seb.nf.sikora@protonmail.com](mailto:seb.nf.sikora@protonmail.com)

Updated 19/10/2021.

What is it?
-------------------------

I created cpp_logic_simulation to explore C++ language features and development methodologies that I do not usually use in my C++ work, due to the resource-constrained nature of the typical target platforms (eg, 512 ~ 2K bytes RAM).

cpp_logic_simulation is a framework for constructing simulations of digital [logic circuits](https://learn.sparkfun.com/tutorials/digital-logic/all). The logic circuits are comprised of *Components*, sub-classed into *Gates* and *Devices*. *Gates* are n-input representations of the [logical operators](https://learn.sparkfun.com/tutorials/digital-logic/all#combinational-logic) NOT, AND, OR, NAND, NOR and XOR. *Gates* can be grouped-together and interconnected within *Devices*. These *Devices* can then be grouped-together and interconnected and nested-within further *Devices*, ad-nauseum. In this way, arbitrarily complex logic circuits can be assembled from the primary *Gate* building blocks. *Devices* can even be created from repeated arrays of simpler building-block *Devices* in a programmatic manner. [See](.sim_src/devices.cpp) `./sim_src/devices.cpp` for examples.

A further sub-class of the *Device* is the *Simulation*, which must always be the top-level *Device*. The *Simulation* can also contain *Clocks* to drive the contained logic circuit, and *Probes*, which can sample and display the logical state of any inputs or outputs of connected *Gates* or *Devices* within the circuit (at any nesting-level). The state of the logic circuit is recalculated automatically in response to any 'live' state changes at it's inputs. However, sampling and displaying state using *Probes* can only occur while the simulation is 'running', with the circuit driven via one or more *Clocks*.

A number of demonstrations using the framework to simulate simple logic circuits are provided here. To run the simulations with maximally verbose output, detailing all of state changes within the circuit at runtime, set the `verbose` and `monitor_on` flags towards the top of the demos to `true`.
* `jk_ff_demo.cpp` - A master-slave JK flip-flop - [more](./sim_doc/jk_ff_demo.md)
* `4_bit_counter_demo.cpp` - A simple 4-bit counter - [more](./sim_doc/4_bit_counter_demo.md)
* `1_bit_register_demo.cpp` - A 1-bit storage register - [more](./sim_doc/1_bit_register_demo.md)
* `n_bit_register_demo.cpp` - A 4-bit register built programmatically from an array of 1-bit registers - [more](./sim_doc/n_bit_register_demo.md)
* `n_x_1_bit_mux_demo.cpp` - An N by 1-bit multiplexor built programmatically
* `n_bit_decoder_demo.cpp` - An N-bit decoder built programmatically
* `n_x_m_bit_mux_demo.cpp` - An N by M-bit multiplexor built programmatically from N by 1-bit muxes and an N-bit decoder

To do anything more interesting than view *Probe* output tables of changing logic levels within the circuit, a way to 'break the fourth wall' and interface with system resources 'outside' of the simulation is required. This is provided by a final *Device* sub-class, the *MagicDevice*. In addition to the usual *Device* functionality, *MagicDevices* contain special custom code (the *MagicEngine*) to interact with system resources outside of the simulation, and interfaces that hook into the simulated operation of the logic-circuit, and vice-versa. This allows us to create *MagicDevices* that, for example, behave as a RAM IC by accessing data contained in an array, a ROM IC by accessing data contained in a text file, or even a UART-like IC communicating with a remote text terminal! See `./sim_src/simple_ram.cpp`, `simple_rom.cpp` and `simple_terminal.cpp` for examples.

Some *MagicDevice* demonstrations are provided here.
* `simple_rom_demo.cpp` - A ROM IC that pulls data from a text file - [more](./sim_doc/simple_rom_demo.md)
* `simple_ram_demo.cpp` - A RAM IC that stores data in an array - [more](./sim_doc/simple_ram_demo.md)
* `simple_terminal_demo.cpp` - A simple UART-like IC and accompanying terminal client - [more](./sim_doc/simple_terminal_demo.md)

Files:
-------------------------
* `.cpp` & `.h` files beginning with `c_...` comprise the class prototypes and definitions that comprise the 'core engine'
* `devices.cpp` & `.h` contain example *Devices* constructed using the framework
* `.cpp` & `.h` files beginning with `simple_...` contain example *MagicDevices* constructed using the framework
* `data.txt` is the data text file to accompany `simple_rom_demo.cpp`
* `utils.cpp` & `.h`, `strnatcmp.cpp` & `.h` contain shared helper functions
* `colors.h` contains macros for adding escape codes for colour terminal text
* `terminal_client.c` contains the simple terminal client to accompany `simple_terminal_demo.cpp`

Running the demos:
-------------------------

To run the demos, compile and link them against the contents of the `./sim_src/` sub-directory using a contemporary C++ compiler. For example, to compile and link the JK flip-flop demo using the GNU C++ compiler on a Linux system, enter the following on the command-line from the main project directory:

`g++ -o jk_ff_demo -I ./sim_src ./sim_src/*.cpp jk_ff_demo.cpp`

then enter `./jk_ff_demo` to run the demo.

Thoughts & limitations:
-------------------------

In no particular order:
* Only logic circuits that are statically-stable state can be simulated.
* I have not written any destructors for any of the classes. As the simulation is 'one-shot', all the objects are created at runtime, the simulation runs, and then the program finishes. For this reason, we don't leak memory. If we wanted to use the simulator as the 'engine' of a larger application in which we wanted to be able to create and destroy components, clocks and probes at will without terminating the program, destructors would be needed for each class to avoid memory leakage.
* All methods & data are public. Although for the most part *Components* use Getters & Setters to access each-other's variables rather than accessing them directly, it would be better to take advantage of the enforced separation of public & private methods and data that C++ allows. 

License:
-------------------------

![LGPLv3 logo](sim_doc/220px-LGPLv3_Logo.png)

cpp_logic_simulation is distributed under the terms of the GNU Lesser General Public License v3.

cpp_logic_simulation is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

cpp_logic_simulation is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with cpp_logic_simulation.  If not, see <https://www.gnu.org/licenses/>.
