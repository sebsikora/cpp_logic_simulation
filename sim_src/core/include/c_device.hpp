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

#ifndef LSIM_CORE_DEVICE_HPP
#define LSIM_CORE_DEVICE_HPP

#include <string>
#include <vector>
#include <mutex>

#include "c_structs.hpp"
#include "c_comp.hpp"

// Compound-logic Device Component sub-class. 
class Device : public Component {
	public:
		Device(Device* parent_device_pointer, std::string const& device_name, std::string const& device_type, std::vector<std::string> in_pin_names,
			std::vector<std::string> out_pin_names, bool monitor_on = false, std::vector<StateDescriptor> in_pin_default_states = {},
			int max_propagations = 0
		);
		virtual ~Device();
		
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
		
		// Device class virtual methods.
		virtual void Build(void) = 0;
		virtual void Solve(void);

		// Device class public methods.
		void CreateInPins(std::vector<std::string> const& pin_names, std::vector<StateDescriptor> pin_default_states);
		void CreateOutPins(std::vector<std::string> const& pin_names);
		void CreateBus(int pin_count, std::string const& pin_name_prefix, Pin::Type type, std::vector<StateDescriptor> in_pin_default_states = {});
		void SetPin(Pin& target_pin, std::vector<StateDescriptor> pin_default_states);
		void AddComponent(Component* new_component_pointer);
		void AddGate(std::string const& component_name, std::string const& component_type, std::vector<std::string> const& in_pin_names, bool monitor_on = false);
		void AddGate(std::string const& component_name, std::string const& component_type, bool monitor_on = false);
		void ChildConnect(std::string const& target_child_component_name, std::vector<std::string> const& connection_parameters);
		void ChildSet(std::string const& target_child_component_name, std::string const& target_pin_name, bool logical_state);
		void ChildPrintPinStates(std::string const& target_child_component_name, int max_levels);
		void ChildPrintInPinStates(std::string const& target_child_component_name);
		void ChildPrintOutPinStates(std::string const& target_child_component_name);
		void ChildMarkOutputNotConnected(std::string const& target_child_component_name, std::string const& target_out_pin_name);
		void Connect(std::string const& origin_pin_name, std::string const& target_component_name, std::string const& target_pin_name = "input");
		void Stabilise(void);
		Component* GetChildComponentPointer(std::string const& target_child_component_name);
		int GetNestingLevel(void);
		int GetInPinCount(void);
		void QueueToPropagatePrimary(Component* component_pointer);
		void PrintInternalPinStates(int max_levels);
		void MarkInnerTerminalsDisconnected(void);
		Component* SearchForComponentPointer(std::string const& target_component_full_name);
		void PurgeChildConnections(Component* target_component_pointer);
		void PurgeChildComponent(std::string const& target_component_name);
		void PurgeAllChildComponents(void);
		void PurgeChildComponentIdentifiers(Component* target_component_pointer);
		bool GetDeletionFlag(void);
		void SetDeletionFlag(bool flag);

	protected:
		// Device class protected methods.
		void QueueToSolve(Device* device_pointer);
		void QueueToPropagateSecondary(Component* component_pointer);
		
		int m_max_propagations;
		bool m_solve_children_in_own_threads = false;
		
	private:
		// Device class private methods.
		void PropagateInputs(void);
		void SubTick(void);
		
		std::vector<Component*> m_components;
		std::mutex m_propagation_lock;
		std::vector<Component*> m_propagate_next_tick = {};
		std::vector<Component*> m_propagate_this_tick = {};
		bool m_solve_this_tick_flag = false;
		std::vector<Device*> m_solve_this_tick = {};
		std::vector<std::vector<ConnectionDescriptor>> m_ports; 			// Maps in- and out-pins to connection descriptors.
		const std::vector<std::string> m_hidden_in_pins = {"true", "false"};
		const std::vector<std::string> m_hidden_out_pins = {"all_stop"};
		std::vector<StateDescriptor> m_in_pin_default_states;
		bool m_deletion_flag = false;

		std::vector<int> m_in_pin_port_indices;
		std::vector<int> m_out_pin_port_indices;
};

#endif	// LSIM_CORE_DEVICE_HPP
