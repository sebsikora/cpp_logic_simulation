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
struct pin {
	enum pin_type {
		NONE = 0,
		IN,
		HIDDEN_IN,
		OUT,
		HIDDEN_OUT
	};
	enum drive_mode {
		DRIVE_IN = 0,
		DRIVE_OUT
	};
	struct drive_state {
		 bool in;
		 bool out;
	};
	
	std::string name;
	pin_type type;
	bool state;
	bool state_changed = false;
	int port_index;
	drive_state drive;
};

struct state_descriptor {
	std::string identifier;
	bool state;
};

struct solver_configuration {
	bool use_threaded_solver;
	int threaded_solve_nesting_level;
};

struct probe_configuration {
	int probe_every_n_ticks;
	int samples_per_row;
	std::vector<std::string> output_characters;
};

struct connection_descriptor {
	Component* target_component_pointer;
	int target_pin_port_index;
};

#endif	// LSIM_CORE_STRUCTS_H
