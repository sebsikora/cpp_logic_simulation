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

Basic example.
-------------------------

Let's dive-in and make something.

[Sequential](https://en.wikipedia.org/wiki/Sequential_logic) digital logic circuits need a way to store state. To this they typically make use of [flip-flops](https://en.wikipedia.org/wiki/Flip-flop_(electronics)), a bi-stable logic circuit that can 'latch' in one of two states, of which there are a number of types.

The simplest flip-flop type is the [SR latch](https://en.wikipedia.org/wiki/Flip-flop_(electronics)#SR_AND-OR_latch). An SR latch can be made in a number of ways, but the pedagologically simplest version comprises one AND gate, one OR gate, and an inverter (NOT 'gate'). Let's simulate one.

```cpp
// sr_latch_demo.cpp

#include "c_core.h"			// Core simulator functionality

int main () {
	bool verbose_flag = false;                     // Set = true to see 'verbose output' on the console.
	Simulation sim("sr_latch", verbose_flag);
	
	// Instantiate gates.
	//
	bool monitor_flag = true;                      // monitor_flag = true to print changes at out pins on the console.
	sim.AddGate("or_0", "or", {"input_0", "input_1"}, monitor_flag);
	sim.AddGate("and_0", "and", {"input_0", "input_1"}, monitor_flag);
	sim.AddGate("not_0", "not", monitor_flag);     // NOT gate only has one in pin "input" by default.

	// Interconnect gates.
	//
	sim.ChildConnect("or_0", {"and_0", "input_0"});
	sim.ChildConnect("not_0", {"and_0", "input_1"});
	sim.ChildConnect("and_0", {"or_0", "input_0"});

	// Connections from simulation.
	//
	sim.Connect("false", "or_0", "input_1");       // Cannot have un-driven in pins so we connect Simulation's
	sim.Connect("false", "not_0");                 // utility pin "false" to both.
	
	sim.Stabilise();                               // Once we have assembled circuit call Stabilise() to
	                                               // settle simulation state.
	
	// or_0:input_1 is 'S', not_0:input is 'R', and_0:output is 'Output'
	
	// 'S' (set).
	sim.ChildSet("or_0", "input_1", true);
	sim.ChildSet("or_0", "input_1", false);
	
	// 'R' (reset).
	sim.ChildSet("not_0", "input", true);
	sim.ChildSet("not_0", "input", false);
	
	// 'S' (set).
	sim.ChildSet("or_0", "input_1", true);
	sim.ChildSet("or_0", "input_1", false);
	
	// 'R' (reset).
	sim.ChildSet("not_0", "input", true);
	sim.ChildSet("not_0", "input", false);
	
	return 0;
}
```

Compile and run:

```
user@home:~/cpp_logic_simulation$ g++ -pthread -Wall -g -O3 -I sim_src/core/ -I sim_src/utils/ -I void_thread_pool/ sim_src/core/c_gate.cpp sim_src/core/c_m_engine.cpp sim_src/core/c_probe.cpp sim_src/core/c_sim.cpp sim_src/core/c_clock.cpp sim_src/core/c_comp.cpp sim_src/core/c_device.cpp sim_src/utils/utils.cpp sim_src/utils/strnatcmp.cpp void_thread_pool/void_thread_pool.cpp sr_latch_demo.cpp -o sr_latch_demo
user@home:~/cpp_logic_simulation$ ./sr_latch_demo

---------------------------- Simulation build started.  ----------------------------

(Simulation verbose output is off)
  MONITOR: test_sim:or_0:or output terminal set to F
  MONITOR: test_sim:not_0:not output terminal set to T

--------------------------- Simulation build completed.  ---------------------------

CHILDSET: Component test_sim:or_0:or terminal input_1 set to T
  MONITOR: test_sim:or_0:or output terminal set to T
  MONITOR: test_sim:and_0:and output terminal set to T

CHILDSET: Component test_sim:or_0:or terminal input_1 set to F

CHILDSET: Component test_sim:not_0:not terminal input set to T
  MONITOR: test_sim:not_0:not output terminal set to F
  MONITOR: test_sim:and_0:and output terminal set to F
  MONITOR: test_sim:or_0:or output terminal set to F

CHILDSET: Component test_sim:not_0:not terminal input set to F
  MONITOR: test_sim:not_0:not output terminal set to T

CHILDSET: Component test_sim:or_0:or terminal input_1 set to T
  MONITOR: test_sim:or_0:or output terminal set to T
  MONITOR: test_sim:and_0:and output terminal set to T

CHILDSET: Component test_sim:or_0:or terminal input_1 set to F

CHILDSET: Component test_sim:not_0:not terminal input set to T
  MONITOR: test_sim:not_0:not output terminal set to F
  MONITOR: test_sim:and_0:and output terminal set to F
  MONITOR: test_sim:or_0:or output terminal set to F

CHILDSET: Component test_sim:not_0:not terminal input set to F
  MONITOR: test_sim:not_0:not output terminal set to T
user@home:~/cpp_logic_simulation$
```

Great! We can see the output responding to the changing input stimulus as we should expect for an SR latch.

Encapsulating our circuit in a *Device*.
-------------------------------------------

We can take our circuit and encapsulate it in a *Device*. We can then easily instantiate multiple copies of it in a single simulation, or re-use it elsewhere.

First of all we need to create a class definition for our device, inheriting from the core *Device* class. We are obliged to define a constructor and a member function Device::Build() within which we will describe the assembly of our new device.

The constructor arguments shown below are the bare-minimum required. We can include as many additional constructor arguments beyond these as needed.

```cpp
// sr_latch.h

#include "c_core.h"			// Core simulator functionality

class SR_Latch : public Device {
	public:
		SR_Latch(Device* parent_device_pointer, std::string name, bool monitor_on = false,
		         std::vector<state_descriptor> input_default_states = {});
		void Build(void);
};
```

Next we create the class implementation for our device. The base *Device* class constructor is called first, among other things it will create the device in pins ("S" & "R") and out pin ("Out"). Next our device constructor calls Build() to create the internal SR latch circuit, and then finally Stabilise() to settle the device's initial internal and external state.

Notice that defining the assembly of the internal circuit is very similar to the previous example. Connections are made from the device's in pins to the in pins of child gates, and from the out pins of child gates to the device's out pin.

```cpp
// sr_latch.cpp

#include "c_core.h"			// Core simulator functionality
#include "sr_latch.h"       // Our new SR_latch device.

SR_Latch::SR_Latch(Device* parent_device_pointer, std::string name, bool monitor_on, std::vector<state_descriptor> input_default_states) 
 : Device(parent_device_pointer, name, "sr_latch", {"S", "R"}, {"Out"}, monitor_on, input_default_states) {
	// Following base class constructor (Device), we call the below overridden Build() method to populate the
	// specific device, then we call the base Stabilise() method to configure initial internal device component state.
	Build();
	Stabilise();
}

void SR_Latch::Build() {
	// Instantiate gates.
	//
	AddGate("or_0", "or", {"input_0", "input_1"}, false);   // monitor_on flag can be omitted,
	AddGate("and_0", "and", {"input_0", "input_1"});        // defaults to false.
	AddGate("not_0", "not");

	// Child gate interconnections.
	//
	ChildConnect("or_0", {"and_0", "input_0"});
	ChildConnect("not_0", {"and_0", "input_1"});
	ChildConnect("and_0", {"or_0", "input_0"});

	// Connections from parent device in pins & to parent device out pin.
	//
	Connect("S", "or_0", "input_1");                        // device's Connect() member function used
	Connect("R", "not_0");                                  // to connect parent in pin to child in pin.
	ChildConnect("and_0", {"parent", "Out"});               // ChildConnect() member function used to connect
	                                                        // child out pin to parent out pin.
}
```

We use our newly defined device in much the same way as in the previous example. In this case we use the simulation's AddComponent() member function to instantiate an SR latch device and add a pointer to it to our simulation's list of child components.

Note we pass a reference to the top-level simulation as the first argument to the device constructor.

We can instantiate the device with desired states applied at it's in pins via the optional in_pin_default_state argument. This is not necessary in this case, but is useful in cases where devices require particular in pin states to settle correctly.

```cpp
// sr_latch_demo_2.cpp

#include "c_core.h"           // Core simulator functionality
#include "sr_latch.h"         // Our new SR_latch device.

int main () {
	bool verbose = false;
	Simulation sim("test_sim", verbose);

	// Add an SR latch device to the top-level simulation.
	//
	bool monitor_on = true
	sim.AddComponent(new SR_Latch(&sim, "sr_latch", monitor_on, {{"S", false}, {"R", false}}));
	
	sim.Stabilise();          // Settle initial device internal and external states.
	
	// 'Set'.
	sim.ChildSet("sr_latch", "S", true);
	sim.ChildSet("sr_latch", "S", false);
	
	// 'Reset'.
	sim.ChildSet("sr_latch", "R", true);
	sim.ChildSet("sr_latch", "R", false);
	
	// 'Set'.
	sim.ChildSet("sr_latch", "S", true);
	sim.ChildSet("sr_latch", "S", false);
	
	// 'Reset'.
	sim.ChildSet("sr_latch", "R", true);
	sim.ChildSet("sr_latch", "R", false);
	
	return 0;
}
```

Compile and run:

```
user@home:~/cpp_logic_simulation$ g++ -pthread -Wall -g -O3 -I sim_src/core/ -I sim_src/devices/ -I sim_src/utils/ -I void_thread_pool/ sim_src/core/c_gate.cpp sim_src/core/c_m_engine.cpp sim_src/core/c_probe.cpp sim_src/core/c_sim.cpp sim_src/core/c_clock.cpp sim_src/core/c_comp.cpp sim_src/core/c_device.cpp sim_src/devices/devices.cpp sim_src/devices/sr_latch.cpp sim_src/utils/utils.cpp sim_src/utils/strnatcmp.cpp void_thread_pool/void_thread_pool.cpp sr_latch_demo_2.cpp -o sr_latch_demo_2
user@home:~/cpp_logic_simulation$ ./sr_latch_demo_2 

---------------------------- Simulation build started.  ----------------------------

(Simulation verbose output is off)

--------------------------- Simulation build completed.  ---------------------------

CHILDSET: Component test_sim:sr_latch:sr_latch terminal S set to T
  MONITOR: Component test_sim:sr_latch:sr_latch input terminal S set to T
  MONITOR: Component test_sim:sr_latch:sr_latch output terminal Out set to T

CHILDSET: Component test_sim:sr_latch:sr_latch terminal S set to F
  MONITOR: Component test_sim:sr_latch:sr_latch input terminal S set to F

CHILDSET: Component test_sim:sr_latch:sr_latch terminal R set to T
  MONITOR: Component test_sim:sr_latch:sr_latch input terminal R set to T
  MONITOR: Component test_sim:sr_latch:sr_latch output terminal Out set to F

CHILDSET: Component test_sim:sr_latch:sr_latch terminal R set to F
  MONITOR: Component test_sim:sr_latch:sr_latch input terminal R set to F

CHILDSET: Component test_sim:sr_latch:sr_latch terminal S set to T
  MONITOR: Component test_sim:sr_latch:sr_latch input terminal S set to T
  MONITOR: Component test_sim:sr_latch:sr_latch output terminal Out set to T

CHILDSET: Component test_sim:sr_latch:sr_latch terminal S set to F
  MONITOR: Component test_sim:sr_latch:sr_latch input terminal S set to F

CHILDSET: Component test_sim:sr_latch:sr_latch terminal R set to T
  MONITOR: Component test_sim:sr_latch:sr_latch input terminal R set to T
  MONITOR: Component test_sim:sr_latch:sr_latch output terminal Out set to F

CHILDSET: Component test_sim:sr_latch:sr_latch terminal R set to F
  MONITOR: Component test_sim:sr_latch:sr_latch input terminal R set to F
user@home:~/cpp_logic_simulation$
```

Great! We can see our SR latch device out pin responding to the changing in pin stimulus as we should expect for an SR latch, just as before.

Nesting *Devices*.
-------------------------

Things get even more interesting once we start nesting *Devices* and *Gates* to form ever-more sophisticated logic circuits. Now that we have defined our SR latch device, let's use it to make a further example.

We can combine both *Devices* and *Gates* in the same circuit, by doing this we will make a quad SR latch with an additional collective reset.

As before, we create a class definition for our device that inherits from the base *Device* class.

```cpp
// quad_sr_latch.h

#include "c_core.h"			// Core simulator functionality

class Quad_SR_Latch : public Device {
	public:
		Quad_SR_Latch(Device* parent_device_pointer, std::string name, bool monitor_on = false,
		              std::vector<state_descriptor> input_default_states = {});
		void Build(void);
};
```

We then create our device class prototype. Note that apart from the type string and list of pin names passed to the base *Device* constructor the device constructor is the same as for our single SR latch.

In our Build() member function, note that we pass the 'this' pointer as the parent_device_pointer first argument to each SR latch device constructor.

We then use the Connect() member function to connect the parent device's in pins to it's child components' in pins, and the ChildConnect() member function to connect child components' out pins to the parent device's out pins.

```cpp
// quad_sr_latch.cpp

#include "c_core.h"	             // Core simulator functionality
#include "sr_latch.h"            // Previously defined SR latch device
#include "quad_sr_latch.h"       // Our new device

Quad_SR_Latch::Quad_SR_Latch(Device* parent_device_pointer, std::string name, bool monitor_on, std::vector<state_descriptor> input_default_states) 
 : Device(parent_device_pointer, name, "quad_sr_latch", {"S_0", "S_1", "S_2", "S_3", "R_0", "R_1", "R_2", "R_3", "R_All"}, {"Out_0", "Out_1", "Out_2", "Out_3"}, monitor_on, input_default_states) {
	// Following base class constructor (Device), we call the below overridden Build() method to populate the
	// specific device, then we call the base Stabilise() method to configure initial internal device component state.
	Build();
	Stabilise();
}

void Quad_SR_Latch::Build() {
	// Instantiate latches.
	//
	AddComponent(new SR_Latch(this, "sr_latch_0"));
	AddComponent(new SR_Latch(this, "sr_latch_1"));
	AddComponent(new SR_Latch(this, "sr_latch_2"));
	AddComponent(new SR_Latch(this, "sr_latch_3"));
	
	// Instantiate gates.
	//
	AddGate("or_0", "or", {"input_0", "input_1"});
	AddGate("or_1", "or", {"input_0", "input_1"});
	AddGate("or_2", "or", {"input_0", "input_1"});
	AddGate("or_3", "or", {"input_0", "input_1"});
	
	Connect("S_0", "sr_latch_0", "S");
	Connect("S_1", "sr_latch_1", "S");
	Connect("S_2", "sr_latch_2", "S");
	Connect("S_3", "sr_latch_3", "S");
	Connect("R_0", "or_0", "input_0");
	Connect("R_1", "or_1", "input_0");
	Connect("R_2", "or_2", "input_0");
	Connect("R_3", "or_3", "input_0");
	Connect("R_All", "or_0", "input_1");
	Connect("R_All", "or_1", "input_1");
	Connect("R_All", "or_2", "input_1");
	Connect("R_All", "or_3", "input_1");
	
	ChildConnect("or_0", {"sr_latch_0", "R"});
	ChildConnect("or_1", {"sr_latch_1", "R"});
	ChildConnect("or_2", {"sr_latch_2", "R"});
	ChildConnect("or_3", {"sr_latch_3", "R"});
	
	ChildConnect("sr_latch_0", {"Out", "parent", "Out_0"});
	ChildConnect("sr_latch_1", {"Out", "parent", "Out_1"});
	ChildConnect("sr_latch_2", {"Out", "parent", "Out_2"});
	ChildConnect("sr_latch_3", {"Out", "parent", "Out_3"});
}
```

Quick demo:

```cpp
// quad_sr_latch_demo.cpp

#include "c_core.h"           // Core simulator functionality
#include "quad_sr_latch.h"    // Our new Quad_SR_latch device.

int main () {
	bool verbose = false;
	Simulation sim("test_sim", verbose);

	// Add an quad SR latch device to the top-level simulation.
	//
	bool monitor_on = true;
	sim.AddComponent(new Quad_SR_Latch(&sim, "quad_sr_latch", monitor_on, {{"S_0", false}, {"S_1", false},
	                 {"S_2", false}, {"S_3", false}, {"R_0", false}, {"R_1", false}, {"R_2", false},{"R_3", false}, {"R_All", false}}));
	
	sim.Stabilise();          // Settle initial device internal and external states.
	
	// 'Reset 0'.
	sim.ChildSet("quad_sr_latch", "R_0", true);
	sim.ChildSet("quad_sr_latch", "R_0", false);
	
	// 'Reset 1'.
	sim.ChildSet("quad_sr_latch", "R_1", true);
	sim.ChildSet("quad_sr_latch", "R_1", false);
	
	// 'Reset 2'.
	sim.ChildSet("quad_sr_latch", "R_2", true);
	sim.ChildSet("quad_sr_latch", "R_2", false);
	
	// 'Reset 3'.
	sim.ChildSet("quad_sr_latch", "R_3", true);
	sim.ChildSet("quad_sr_latch", "R_3", false);
	
	// 'Set 0'.
	sim.ChildSet("quad_sr_latch", "S_0", true);
	sim.ChildSet("quad_sr_latch", "S_0", false);
	
	// 'Set 1'.
	sim.ChildSet("quad_sr_latch", "S_1", true);
	sim.ChildSet("quad_sr_latch", "S_1", false);
	
	// 'Set 2'.
	sim.ChildSet("quad_sr_latch", "S_2", true);
	sim.ChildSet("quad_sr_latch", "S_2", false);
	
	// 'Set 3'.
	sim.ChildSet("quad_sr_latch", "S_3", true);
	sim.ChildSet("quad_sr_latch", "S_3", false);
	
	// 'Reset All'.
	sim.ChildSet("quad_sr_latch", "R_All", true);
	sim.ChildSet("quad_sr_latch", "R_All", false);
	
	return 0;
}
```

Compile and run:

```
user@home:~/cpp_logic_simulation$ g++ -pthread -Wall -g -O3 -I sim_src/core/ -I sim_src/devices/ -I sim_src/utils/ -I void_thread_pool/ sim_src/core/c_gate.cpp sim_src/core/c_m_engine.cpp sim_src/core/c_probe.cpp sim_src/core/c_sim.cpp sim_src/core/c_clock.cpp sim_src/core/c_comp.cpp sim_src/core/c_device.cpp sim_src/devices/devices.cpp sim_src/devices/quad_sr_latch.cpp sim_src/devices/sr_latch.cpp sim_src/utils/utils.cpp sim_src/utils/strnatcmp.cpp void_thread_pool/void_thread_pool.cpp quad_sr_latch_demo.cpp -o quad_sr_latch_demo
user@home:~/cpp_logic_simulation$ ./quad_sr_latch_demo

---------------------------- Simulation build started.  ----------------------------

(Simulation verbose output is off)
  MONITOR: Component test_sim:quad_sr_latch:quad_sr_latch output terminal Out_3 set to T

--------------------------- Simulation build completed.  ---------------------------

CHILDSET: Component test_sim:quad_sr_latch:quad_sr_latch terminal R_0 set to T
  MONITOR: Component test_sim:quad_sr_latch:quad_sr_latch input terminal R_0 set to T

CHILDSET: Component test_sim:quad_sr_latch:quad_sr_latch terminal R_0 set to F
  MONITOR: Component test_sim:quad_sr_latch:quad_sr_latch input terminal R_0 set to F

CHILDSET: Component test_sim:quad_sr_latch:quad_sr_latch terminal R_1 set to T
  MONITOR: Component test_sim:quad_sr_latch:quad_sr_latch input terminal R_1 set to T

CHILDSET: Component test_sim:quad_sr_latch:quad_sr_latch terminal R_1 set to F
  MONITOR: Component test_sim:quad_sr_latch:quad_sr_latch input terminal R_1 set to F

CHILDSET: Component test_sim:quad_sr_latch:quad_sr_latch terminal R_2 set to T
  MONITOR: Component test_sim:quad_sr_latch:quad_sr_latch input terminal R_2 set to T

CHILDSET: Component test_sim:quad_sr_latch:quad_sr_latch terminal R_2 set to F
  MONITOR: Component test_sim:quad_sr_latch:quad_sr_latch input terminal R_2 set to F

CHILDSET: Component test_sim:quad_sr_latch:quad_sr_latch terminal R_3 set to T
  MONITOR: Component test_sim:quad_sr_latch:quad_sr_latch input terminal R_3 set to T
  MONITOR: Component test_sim:quad_sr_latch:quad_sr_latch output terminal Out_3 set to F

CHILDSET: Component test_sim:quad_sr_latch:quad_sr_latch terminal R_3 set to F
  MONITOR: Component test_sim:quad_sr_latch:quad_sr_latch input terminal R_3 set to F

CHILDSET: Component test_sim:quad_sr_latch:quad_sr_latch terminal S_0 set to T
  MONITOR: Component test_sim:quad_sr_latch:quad_sr_latch input terminal S_0 set to T
  MONITOR: Component test_sim:quad_sr_latch:quad_sr_latch output terminal Out_0 set to T

CHILDSET: Component test_sim:quad_sr_latch:quad_sr_latch terminal S_0 set to F
  MONITOR: Component test_sim:quad_sr_latch:quad_sr_latch input terminal S_0 set to F

CHILDSET: Component test_sim:quad_sr_latch:quad_sr_latch terminal S_1 set to T
  MONITOR: Component test_sim:quad_sr_latch:quad_sr_latch input terminal S_1 set to T
  MONITOR: Component test_sim:quad_sr_latch:quad_sr_latch output terminal Out_1 set to T

CHILDSET: Component test_sim:quad_sr_latch:quad_sr_latch terminal S_1 set to F
  MONITOR: Component test_sim:quad_sr_latch:quad_sr_latch input terminal S_1 set to F

CHILDSET: Component test_sim:quad_sr_latch:quad_sr_latch terminal S_2 set to T
  MONITOR: Component test_sim:quad_sr_latch:quad_sr_latch input terminal S_2 set to T
  MONITOR: Component test_sim:quad_sr_latch:quad_sr_latch output terminal Out_2 set to T

CHILDSET: Component test_sim:quad_sr_latch:quad_sr_latch terminal S_2 set to F
  MONITOR: Component test_sim:quad_sr_latch:quad_sr_latch input terminal S_2 set to F

CHILDSET: Component test_sim:quad_sr_latch:quad_sr_latch terminal S_3 set to T
  MONITOR: Component test_sim:quad_sr_latch:quad_sr_latch input terminal S_3 set to T
  MONITOR: Component test_sim:quad_sr_latch:quad_sr_latch output terminal Out_3 set to T

CHILDSET: Component test_sim:quad_sr_latch:quad_sr_latch terminal S_3 set to F
  MONITOR: Component test_sim:quad_sr_latch:quad_sr_latch input terminal S_3 set to F

CHILDSET: Component test_sim:quad_sr_latch:quad_sr_latch terminal R_All set to T
  MONITOR: Component test_sim:quad_sr_latch:quad_sr_latch input terminal R_All set to T
  MONITOR: Component test_sim:quad_sr_latch:quad_sr_latch output terminal Out_0 set to F
  MONITOR: Component test_sim:quad_sr_latch:quad_sr_latch output terminal Out_1 set to F
  MONITOR: Component test_sim:quad_sr_latch:quad_sr_latch output terminal Out_2 set to F
  MONITOR: Component test_sim:quad_sr_latch:quad_sr_latch output terminal Out_3 set to F

CHILDSET: Component test_sim:quad_sr_latch:quad_sr_latch terminal R_All set to F
  MONITOR: Component test_sim:quad_sr_latch:quad_sr_latch input terminal R_All set to F
user@home:~/cpp_logic_simulation$
```

Great!

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
