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

#ifndef LSIM_CORE_GATE_HPP
#define LSIM_CORE_GATE_HPP

#include <string>
#include <vector>

#include "c_structs.hpp"
#include "c_comp.hpp"

class Device;

// Logic Gate Component sub-class.
class Gate : public Component {
	public:
		virtual ~Gate();
		
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

	protected:
		void Configure(Device* parent_device_pointer, std::string const& gate_name, std::string const& gate_type,
					   std::vector<std::string> in_pin_names, bool monitor_on);

		int m_in_pin_count;
		
	private:
		virtual bool Operate(void) = 0;
		void Evaluate(void);
		
		int m_out_pin_port_index;
		std::vector<ConnectionDescriptor> m_connections;
};

#endif // LSIM_CORE_GATE_HPP
