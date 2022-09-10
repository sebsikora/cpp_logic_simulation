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

#ifndef LSIM_CORE_GATE_HPP
#define LSIM_CORE_GATE_HPP

#include <string>
#include <vector>

#include "c_structs.hpp"
#include "c_comp.hpp"

class Device;

// This typedef defines the type 'pointer to a Gate class member function that takes a vector of pin
// structs as arguments and returns an output bool. This allows us to *dramatically* simplify the code
// for declaring such function pointers and member functions that take them as arguments and/or return them.
typedef bool (Gate::*operator_pointer)(std::vector<pin> const&);

// Logic Gate Component sub-class.
class Gate : public Component {
	public:
		Gate(Device* parent_device_pointer, std::string const& gate_name, std::string const& gate_type,
			std::vector<std::string> in_pin_names = {}, bool monitor_on = false
		);
		~Gate();
		
		// Override Component virtual methods.
		void Initialise(void) override;
		void Connect(std::vector<std::string> connection_parameters) override;
		void Set(const int pin_port_index, const bool state_to_set) override;
		void Propagate(void) override;
		void PrintPinStates(int max_levels) override;
		void ReportUnConnectedPins(void) override;
		void Reset(void) override;
		void PurgeComponent(void) override;
		void PurgeInboundConnections(Component* target_component_pointer) override;
		void PurgeOutboundConnections(void) override;
		
	private:
		void Evaluate(void);
		operator_pointer GetOperatorPointer(std::string const& operator_name);
		bool OperatorAnd(std::vector<pin> const& pins);
		bool OperatorNand(std::vector<pin> const& pins);
		bool OperatorOr(std::vector<pin> const& pins);
		bool OperatorNor(std::vector<pin> const& pins);
		bool OperatorNot(std::vector<pin> const& pins);
		
		int m_out_pin_port_index;
		operator_pointer m_operator_function_pointer;
		std::vector<connection_descriptor> m_connections;
};

#endif // LSIM_CORE_GATE_HPP
