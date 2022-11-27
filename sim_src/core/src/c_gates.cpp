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

#include <vector>
#include <string>

#include "c_gates.hpp"

class Device;

AndGate::AndGate(Device* parent_device_pointer, std::string const& gate_name, std::vector<std::string> in_pin_names, bool monitor_on) {
	Configure(parent_device_pointer, gate_name, "and", in_pin_names, monitor_on);
}

inline bool AndGate::Operate(void) {
	bool output = true;
	for (int i = 0; i < m_in_pin_count; ++i) {
		output &= m_pins[i].state;
	}
	return output;
}

NandGate::NandGate(Device* parent_device_pointer, std::string const& gate_name, std::vector<std::string> in_pin_names, bool monitor_on) {
	Configure(parent_device_pointer, gate_name, "nand", in_pin_names, monitor_on);
}

inline bool NandGate::Operate(void) {
	bool output = true;
	for (int i = 0; i < m_in_pin_count; ++i) {
		output &= m_pins[i].state;
	}
	return !output;
}

OrGate::OrGate(Device* parent_device_pointer, std::string const& gate_name, std::vector<std::string> in_pin_names, bool monitor_on) {
	Configure(parent_device_pointer, gate_name, "or", in_pin_names, monitor_on);
}

inline bool OrGate::Operate(void) {
	bool output = false;
	for (int i = 0; i < m_in_pin_count; ++i) {
		output |= m_pins[i].state;
	}
	return output;
}

NorGate::NorGate(Device* parent_device_pointer, std::string const& gate_name, std::vector<std::string> in_pin_names, bool monitor_on) {
	Configure(parent_device_pointer, gate_name, "nor", in_pin_names, monitor_on);
}

inline bool NorGate::Operate(void) {
	bool output = false;
	for (int i = 0; i < m_in_pin_count; ++i) {
		output |= m_pins[i].state;
	}
	return !output;
}

Inverter::Inverter(Device* parent_device_pointer, std::string const& gate_name, bool monitor_on) {
	Configure(parent_device_pointer, gate_name, "not", {"input"}, monitor_on);
}

inline bool Inverter::Operate(void) {
	return !m_pins[0].state;
}
