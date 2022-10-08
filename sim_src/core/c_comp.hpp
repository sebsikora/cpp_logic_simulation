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

#ifndef LSIM_CORE_COMP_HPP
#define LSIM_CORE_COMP_HPP

#include <string>
#include <vector>

#include "c_structs.hpp"

class Device;
class Simulation;

// Base Component class.
class Component {
	public:
		Component() { }
		virtual ~Component();
				
		// Component class virtual methods.
		virtual void Initialise(void) = 0;
		virtual void Connect(std::vector<std::string> connection_parameters) = 0;
		virtual void Set(const int pin_port_index, const bool state_to_set) = 0;
		virtual void Propagate(void) = 0;
		virtual void PrintPinStates(int max_levels) = 0;
		virtual void ReportUnConnectedPins(void) = 0;
		virtual void Reset(void) = 0;
		virtual void PurgeComponent(void) = 0;
		virtual void PurgeInboundConnections(Component* target_component_pointer) = 0;
		virtual void PurgeOutboundConnections(void) = 0;
		
		std::string GetName(void);
		std::string GetFullName(void);
		bool GetDeviceFlag(void);
		std::string GetComponentType(void);
		int GetLocalComponentIndex(void);
		void SetLocalComponentIndex(int new_local_component_index);
		Simulation* GetTopLevelSimPointer(void);
		bool GetPinState(const int pin_port_index);
		std::string GetPinName(int pin_port_index);
		std::vector<std::string> GetSortedInPinNames(void);
		std::vector<std::string> GetSortedOutPinNames(void);
		pin::pin_type GetPinType(int pin_port_index);
		pin::pin_type GetPinType(std::string const& pin_name);
		int GetPinPortIndex(std::string const& pin_name);
		bool CheckIfPinExists(std::string const& target_pin_name);
		pin::drive_state* CheckIfPinDriven(int pin_port_index);
		void SetPinDrivenFlag(int pin_port_index, pin::drive_mode mode, bool state_to_set);
		void PrintInPinStates(void);
		void PrintOutPinStates(void);

	protected:
		void GenerateFullName(std::string &workingString);
		
		int m_nesting_level = 0;
		bool m_device_flag;
		std::string m_name;
		int m_CUID;
		int m_local_component_index = 0;
		std::string m_component_type;
		Simulation* m_top_level_sim_pointer;
		Device* m_parent_device_pointer;
		std::vector<pin> m_pins;
		bool m_queued_for_propagation = false;
};

#endif	// LSIM_CORE_COMP_HPP
