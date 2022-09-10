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

#ifndef LSIM_CORE_DEVICE_HPP
#define LSIM_CORE_DEVICE_HPP

#include <string>
#include <vector>
#include <mutex>

#include "c_structs.hpp"
#include "c_comp.hpp"

class MagicEngine;

// Compound-logic Device Component sub-class. 
class Device : public Component {
	public:
		Device(Device* parent_device_pointer, std::string const& device_name, std::string const& device_type, std::vector<std::string> in_pin_names,
			std::vector<std::string> out_pin_names, bool monitor_on = false, std::vector<state_descriptor> in_pin_default_states = {},
			int max_propagations = 0
		);
		~Device();
		
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

		// Device class public methods.
		void CreateInPins(std::vector<std::string> const& pin_names, std::vector<state_descriptor> pin_default_states);
		void CreateOutPins(std::vector<std::string> const& pin_names);
		void CreateBus(int pin_count, std::string const& pin_name_prefix, int pin_direction, std::vector<state_descriptor> in_pin_default_states = {});
		void SetPin(pin& target_pin, std::vector<state_descriptor> pin_default_states);
		void AddComponent(Component* new_component_pointer);
		void AddGate(std::string const& component_name, std::string const& component_type, std::vector<std::string> const& in_pin_names, bool monitor_on = false);
		void AddGate(std::string const& component_name, std::string const& component_type, bool monitor_on = false);
		void AddMagicEventTrap(std::string const& target_pin_name, std::vector<bool> const& state_change,
			std::vector<human_writable_magic_event_co_condition> const& hw_co_conditions, int incantation);
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
		int GetNewLocalComponentIndex(void);
		int GetLocalComponentCount(void);
		int GetInPinCount(void);
		void AppendChildPropagationIdentifier(const int propagation_identifier);
		void QueueToPropagate(const int propagation_identifier);
		void PrintInternalPinStates(int max_levels);
		void MarkInnerTerminalsDisconnected(void);
		Component* SearchForComponentPointer(std::string const& target_component_full_name);
		void PurgeChildConnections(Component* target_component_pointer);
		void PurgeChildComponent(std::string const& target_component_name);
		void PurgeAllChildComponents(void);
		void PurgeChildComponentIdentifiers(Component* target_component_pointer);
		void CreateChildFlags(void);
		bool GetDeletionFlag(void);
		int GetMessageBranchID(void);
		
	private:
		// Device class private methods.
		void SubTick(const int index);
		
		std::vector<component_descriptor> m_components;
		std::vector<int> m_devices;
		std::mutex m_propagation_lock;
		std::vector<int> m_propagate_next_tick = {};
		std::vector<bool> m_propagate_next_tick_flags = {};
		std::vector<int> m_propagate_this_tick = {};
		std::vector<bool> m_propagate_this_tick_flags = {};
		bool m_buffered_propagation = false;
		bool m_solve_this_tick_flag = false;
		std::vector<int> m_solve_this_tick = {};
		std::vector<std::vector<connection_descriptor>> m_ports; 			// Maps in- and out-pins to connection descriptors.
		const std::vector<std::string> m_hidden_in_pins = {"true", "false"};
		const std::vector<std::string> m_hidden_out_pins = {"all_stop"};
		std::vector<state_descriptor> m_in_pin_default_states;
		int m_message_branch_id = 0;
		
	protected:
		// Device class protected methods.
		void Solve(const bool threaded_solve, const int branch_id);
		void QueueToSolve(const int local_component_identifier);
		void PropagateInputs(void);
		
		int m_max_propagations;
		bool m_deletion_flag = false;
		bool m_magic_device_flag = false;
		MagicEngine* m_magic_engine_pointer;
		std::vector<bool> m_magic_pin_flag;
		bool m_solve_children_in_own_threads = false;
};

#endif	// LSIM_CORE_DEVICE_HPP
