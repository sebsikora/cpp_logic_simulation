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

#ifndef LSIM_CORE_GATES_HPP
#define LSIM_CORE_GATES_HPP

#include "c_gate.hpp"

class Device;

class AndGate : public Gate {
	public:
		AndGate(Device* parent_device_pointer, std::string const& gate_name, std::vector<std::string> in_pin_names, bool monitor_on = false);
		virtual ~AndGate() { }

	private:
		virtual bool Operate(void) final override;
};

class NandGate : public Gate {
	public:
		NandGate(Device* parent_device_pointer, std::string const& gate_name, std::vector<std::string> in_pin_names, bool monitor_on = false);
		virtual ~NandGate() { }

	private:
		virtual bool Operate(void) final override;
};

class OrGate : public Gate {
	public:
		OrGate(Device* parent_device_pointer, std::string const& gate_name, std::vector<std::string> in_pin_names, bool monitor_on = false);
		virtual ~OrGate() { }

	private:
		virtual bool Operate(void) final override;
};

class NorGate : public Gate {
	public:
		NorGate(Device* parent_device_pointer, std::string const& gate_name, std::vector<std::string> in_pin_names, bool monitor_on = false);
		virtual ~NorGate() { }

	private:
		virtual bool Operate(void) final override;
};

class Inverter : public Gate {
	public:
		Inverter(Device* parent_device_pointer, std::string const& gate_name, bool monitor_on = false);
		virtual ~Inverter() { }

	private:
		virtual bool Operate(void) final override;
};

#endif	// LSIM_CORE_GATES_HPP
