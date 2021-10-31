/*
	
    This file is part of cpp_logic_simulation, a simple C++ framework for the simulation of digital logic circuits.
    Copyright (C) 2021 Dr Seb N.F. Sikora
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
#ifndef LSIM_STRUCTS_H
#define LSIM_STRUCTS_H

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
class MagicEngine;

// Define data structures.
struct pin {
	std::string name;
	int direction;
	bool state;
	bool state_changed;
	int port_index;
	std::vector<bool> drive;
};

struct component_descriptor {
	std::string component_name;
	std::string component_full_name;
	Component* component_pointer;
};

struct state_descriptor {
	std::string identifier;
	bool state;
};

struct connection_descriptor {
	Component* target_component_pointer;
	int target_pin_port_index;
};

struct clock_descriptor {
	std::string clock_name;
	Clock* clock_pointer;
};

struct probe_descriptor {
	std::string probe_name;
	Probe* probe_pointer;
};

struct magic_engine_descriptor {
	bool operator==(magic_engine_descriptor const& rhs) const {
		return ((this->magic_engine_identifier == rhs.magic_engine_identifier) && (this->magic_engine_pointer == rhs.magic_engine_pointer));
	}
	bool operator!=(magic_engine_descriptor const& rhs) const {
		return ((this->magic_engine_identifier != rhs.magic_engine_identifier) || (this->magic_engine_pointer != rhs.magic_engine_pointer));
	}
	std::string magic_engine_identifier;
	MagicEngine* magic_engine_pointer;
};

struct magic_event_co_condition {
	bool operator==(magic_event_co_condition const& rhs) const {
		return ((this->pin_port_index == rhs.pin_port_index) && (this->pin_state == rhs.pin_state));
	}
	int pin_port_index; 
	bool pin_state;
};

struct human_writable_magic_event_co_condition {
	bool operator==(human_writable_magic_event_co_condition const& rhs) const {
		return ((this->pin_name == rhs.pin_name) && (this->pin_state == rhs.pin_state));
	}
	std::string pin_name;
	bool pin_state;
};

struct magic_event {
	bool operator==(magic_event const& rhs) const {
		return ((this->target_pin_port_index == rhs.target_pin_port_index) &&
				(this->state_change == rhs.state_change) &&
				(this->co_conditions == rhs.co_conditions) &&
				(this->incantation == rhs.incantation));
	}
	int target_pin_port_index;
	std::vector<bool> state_change;
	std::vector<magic_event_co_condition> co_conditions;
	int incantation;
};

#endif
