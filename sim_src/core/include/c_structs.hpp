/*
	
    This file is part of cpp_logic_simulation, a simple C++ framework for the simulation of digital logic circuits.
    Copyright (C) 2022 Dr Seb N.F. Sikora
    seb.nf.sikora@protonmail.com
	
    cpp_logic_simulation is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    cpp_logic_simulation is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with cpp_logic_simulation.  If not, see <https://www.gnu.org/licenses/>.

*/

// Include guard for this header.
#ifndef LSIM_CORE_STRUCTS_H
#define LSIM_CORE_STRUCTS_H

// Includes for this header.
#include <string>					// std::string.
#include <vector>					// std::vector

// Forward declarations for struct definitions below.
class Component;
class Gate;
class Device;
class Simulation;
class Clock;
class Probe;

// Define data structures.
struct Pin {
	enum Type {
		NONE = 0,
		IN,
		HIDDEN_IN,
		OUT,
		HIDDEN_OUT
	};
	enum DriveDirection {
		DRIVE_IN = 0,
		DRIVE_OUT
	};
	struct Driven {
		 bool in;
		 bool out;
	};
	
	std::string name;
	Type type;
	bool state;
	bool state_changed = false;
	int port_index;
	Driven driven;
};

struct StateDescriptor {
	std::string identifier;
	bool state;
};

struct SolverConfiguration {
	bool use_threaded_solver;
	int threaded_solve_nesting_level;
};

struct ProbeConfiguration {
	struct OutputChars {
		char low;
		char high;
	};
	int probe_every_n_ticks;
	int samples_per_row;
	OutputChars output_characters;
};

struct ConnectionDescriptor {
	Component* target_component_pointer;
	int target_pin_port_index;
};

#endif	// LSIM_CORE_STRUCTS_H
