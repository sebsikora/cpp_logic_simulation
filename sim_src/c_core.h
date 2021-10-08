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

#ifndef LSIM_CORE_H
#define LSIM_CORE_H

#include <functional>				// std::hash
#include <string>					// std::string.
#include <iostream>					// std::cout, std::endl.
#include <vector>					// std::vector
#include <unordered_map>			// std::unordered_map

#include <termios.h>

// Forward declarations for struct and class definitions below.
class Component;
class Gate;
class Device;
class Simulation;
class Clock;
class Probe;
class MagicEngine;

struct connection_descriptor {
	Component* target_component;
	std::size_t target_pin_name_hash;
};

struct pin {
	std::string name;
	int direction;
	bool state;
	bool state_changed;
};

//~struct sorted_pin_identifiers {
	//~struct in {
		//~std::vector<std::string> names;
		//~std::vector<std::string> hashes;
	//~};
	//~struct out {
		//~std::vector<std::size_t> names;
		//~std::vector<std::size_t> hashes;
	//~};
//~};

// This typedef defines the type 'pointer to a Gate class member function that takes a unordered_map of state 
// structs to strings as arguments and returns a state struct. This allows us to *dramatically*
// simplify the code for declaring such function pointers and member functions that take them as
// arguments and/or return them.
typedef bool (Gate::*operator_pointer)(std::unordered_map<std::size_t, bool> const&);

// Class definitions proper.
// Base logic component class.
class Component {
	public:
		// Constructor.
		Component() {}
		// Methods.
		std::string GetName(void);
		std::size_t GetNameHash(void);
		bool GetDeviceFlag(void);
		std::string GetFullName(void);
		std::string GetComponentType(void);
		Simulation* GetTopLevelSimPointer(void);
		bool GetInPinState(std::size_t pin_name_hash);
		bool GetOutPinState(std::size_t pin_name_hash);
		std::vector<std::string> GetInPinNames(void);
		std::vector<std::string> GetOutPinNames(void);
		std::vector<std::size_t> GetInPinNameHashes(void);
		std::string GetInPinName(std::size_t in_pin_name_hash);
		std::string GetOutPinName(std::size_t out_pin_name_hash);
		std::vector<int> GetPinDirections(std::vector<std::size_t> const& pin_name_hashes);
		void PrintInPinStates(void);
		void PrintOutPinStates(void);
		// Virtual methods.
		virtual void Initialise(void) = 0;
		virtual void Connect(std::string const& origin_pin_name, std::string const& target_component_name, std::string const& target_pin_name) = 0;
		virtual void Set(std::size_t terminal_name_hash, bool state_to_set) = 0;
		virtual void Propagate(void) = 0;
		virtual void PrintPinStates(int max_levels) = 0;
		virtual void MakeProbable(void) = 0;
		
		// Data.
		int m_nesting_level;
		bool m_device_flag;
		std::string m_name;
		std::size_t m_name_hash;
		std::string m_full_name;
		int m_CUID;
		std::string m_component_type;
		Simulation* m_top_level_sim_pointer;
		Device* m_parent_device_pointer;
		std::unordered_map<std::size_t, int> m_pin_directions;
		std::unordered_map<std::size_t, bool> m_in_pin_states;
		std::unordered_map<std::size_t, bool> m_out_pin_states;
		std::vector<std::string> m_sorted_in_pin_names;
		std::vector<std::string> m_sorted_out_pin_names;
		std::vector<std::size_t> m_sorted_in_pin_name_hashes;
		std::vector<std::size_t> m_sorted_out_pin_name_hashes;
		bool m_monitor_on;
		static bool mg_verbose_output_flag;
};

// Basic logic-gate component subclass.
class Gate : public Component {
	public:
		// Constructor.
		Gate(Device* parent_device_pointer, std::string const& gate_name, std::string const& gate_type, std::vector<std::string> in_pin_names, bool monitor_on);
		// Override methods common to Components.
		void Initialise(void) override;
		void PrintPinStates(int max_levels) override;
		void Connect(std::string const& origin_pin_name, std::string const& target_component_name, std::string const& target_pin_name) override;
		void Set(std::size_t pin_name_hash, bool state_to_set) override;
		void Propagate(void) override;
		void MakeProbable(void) override;
		// Gate-specific methods.
		void Evaluate(void);
		Component* GetSiblingComponentPointer(std::string const& target_sibling_component_name);
		operator_pointer GetOperatorPointer(std::string const& operator_name);
		bool OperatorAnd(std::unordered_map<std::size_t, bool> const& in_pin_states);
		bool OperatorNand(std::unordered_map<std::size_t, bool> const& in_pin_states);
		bool OperatorOr(std::unordered_map<std::size_t, bool> const& in_pin_states);
		bool OperatorNor(std::unordered_map<std::size_t, bool> const& in_pin_states);
		bool OperatorNot(std::unordered_map<std::size_t, bool> const& in_pin_states);
		
		// Data.
		operator_pointer m_operator_function_pointer;
		std::unordered_map<std::size_t, connection_descriptor> m_connections;
};

// Complex composite device component subclass.
struct magic_event_co_condition {
	std::size_t pin_name_hash; 
	bool pin_state;
};

struct human_writable_magic_event_co_condition {
	std::string pin_name;
	bool pin_state;
};

class Device : public Component {
	public:
		// Constructor.
		Device(Device* parent_device_pointer, std::string const& device_name, std::string const& device_type, std::vector<std::string> in_pin_names, std::vector<std::string> const& out_pin_names, bool monitor_on, std::unordered_map<std::string, bool> const& in_pin_default_states, int max_propagations = 0);
		// Override methods common to Components.
		void Initialise(void) override;
		void PrintPinStates(int max_levels) override;
		void Connect(std::string const& origin_pin_name, std::string const& target_component_name, std::string const& target_pin_name) override;
		void Set(std::size_t pin_name_hash, bool state_to_set) override;
		void Propagate(void) override;
		void MakeProbable(void) override;
		// Device-specific methods.
		virtual void Build(void);
		
		void CreateInPins(std::vector<std::string> const& pin_names, std::unordered_map<std::string, bool> pin_default_states);
		void CreateOutPins(std::vector<std::string> const& pin_names);
		void AddComponent(Component* new_component_pointer);
		void AddGate(std::string const& component_name, std::string const& component_type, std::vector<std::string> const& in_pin_names, bool monitor_on);
		void AddMagicEventTrap(std::string const& target_pin_name, std::vector<bool> const& state_change, std::vector<human_writable_magic_event_co_condition> const& hw_co_conditions, std::string const& incantation);
		void ChildConnect(std::string const& target_child_component, std::vector<std::string> const& connection_parameters);
		void ChildSet(std::string const& target_child_component_name, std::string const& target_pin_name, bool logical_state);
		void ChildPrintPinStates(std::string const& target_child_component_name, int max_levels);
		void ChildPrintInPinStates(std::string const& target_child_component_name);
		void ChildPrintOutPinStates(std::string const& target_child_component_name);
		void ChildMakeProbable(std::string const& target_child_component_name);
		void Stabilise(void);
		void SubTick(int index);
		void Solve(void);
		int GetNestingLevel(void);
		Component* GetChildComponentPointer(std::string const& target_child_component_name);
		void AddToPropagateNextTick(std::size_t propagation_identifier);
		bool CheckIfQueuedToPropagateThisTick(std::size_t propagation_identifier);
		void PrintInternalPinStates(int max_levels);
		
		// Data.
		int m_max_propagations;
		std::unordered_map<std::size_t, bool> m_in_pin_state_changed;
		std::unordered_map<std::size_t, bool> m_out_pin_state_changed;
		std::unordered_map<std::size_t, Component*> m_components;
		std::unordered_map<std::size_t, bool> m_propagate_next_tick;
		std::unordered_map<std::size_t, bool> m_yet_to_propagate_this_tick;
		std::unordered_map<std::size_t, std::unordered_map<std::size_t, connection_descriptor>> m_ports;
		bool m_magic_device_flag;
		MagicEngine* m_magic_engine_pointer;
		std::vector<std::string> m_hidden_in_pins = {"true", "false"};
};

// Top-level simulation device subclass.
class Simulation : public Device {
	public:
		// Constructor.
		Simulation(std::string const& simulation_name, int max_propagations, bool verbose_output_flag);
		// Methods.
		void Run(int number_of_ticks = 0, bool restart_flag = true, bool verbose_debug_flag = false, bool print_probes_flag = false);
		void AddClock(std::string const& clock_name, std::vector<bool> const& toggle_pattern, bool monitor_on);
		void ClockConnect(std::string const& target_clock, std::string const& target_component_name, std::string const& target_terminal_name);
		void AddProbe(std::string const& probe_name, std::string const& target_component_full_name, std::vector<std::string> const& target_pin_names, std::string const& trigger_clock_name);
		void AddToProbableDevices(std::string const& target_component_full_name, Component* target_component_pointer);
		void AddToMagicEngines(std::string const& target_magic_engine_identifier, MagicEngine* target_magic_engine_pointer);
		int GetTopLevelMaxPropagations(void);
		int GetNewCUID(void);
		Clock* GetClockPointer(std::string const& clock_name);
		Component* GetProbableComponentPointer(std::string const& target_component_full_name);
		int GetTopLevelComponentCount(void);
		void EnableTerminalRawIO(bool raw_flag);
		char CheckForCharacter(void);
		bool IsSimulationRunning(void);
		void StopSimulation(void);
		void ShutDown(void);
		void CheckProbeTriggers(void);
		
		// Data.
		std::unordered_map<std::string, Component*> m_probable_components;
		std::unordered_map<std::string, Probe*> m_probes;
		std::unordered_map<std::size_t, Clock*> m_clocks;
		std::unordered_map<std::string, MagicEngine*> m_magic_engines;
		termios m_old_term_io_settings;
		bool m_simulation_running;
		int m_global_tick_index;
		int m_next_new_CUID;
};

// Clock generator utility class.
class Clock {
	public:
		// Constructor.
		Clock(Device* parent_device_pointer, std::string const& clock_name, std::vector<bool> toggle_pattern, bool monitor_on);
		// Methods.
		void Connect(std::string const& target_component_name, std::string const& pin_name);
		void Propagate(void);
		void Tick(void);
		void Reset(void);
		void AddToProbeList(std::string const& probe_identifier, Probe* probe_pointer);
		void TriggerProbes(void);
		bool GetTickedFlag(void);
		
		// Data.
		Device* m_parent_device_pointer;
		std::string m_name;
		std::size_t m_name_hash;
		std::vector<bool> m_toggle_pattern;
		bool m_monitor_on;
		std::unordered_map<std::size_t, connection_descriptor> m_connections;
		std::unordered_map<std::size_t, bool> m_out_pin_states;
		std::vector<bool> m_state_history;
		int m_index;
		int m_sub_index;
		std::unordered_map<std::string, Probe*> m_probes;
		bool m_ticked_flag;
};

// Clock-triggered logic probe utility class.
class Probe {
	public:
		// Constructor.
		Probe(Simulation* top_level_device_pointer, std::string const& probe_name, std::string const& target_component_full_name, std::vector<std::string> const& target_pins, std::string const& trigger_clock_name);
		// Methods.
		void Sample(int index);
		void Reset(void);
		void PrintSamples(void);
		
		// Data.
		Simulation* m_top_level_sim_pointer;
		std::string m_name;
		std::string m_target_component_full_name;
		Component* m_target_component_pointer;
		std::vector<std::size_t> m_target_pins;
		std::vector<int> m_target_pin_directions;
		std::string m_trigger_clock_name;
		Clock* m_trigger_clock_pointer;
		std::vector<int> m_timestamps;
		std::vector<std::vector<bool>> m_samples;
};

// Comment goes here...
struct magic_event {
	std::size_t target_pin_name_hash;
	std::vector<bool> state_change;
	std::vector<magic_event_co_condition> co_conditions;
	std::string incantation;
};

class MagicEngine {
	public:
		// Constructor.
		MagicEngine(Device* parent_device_pointer);
		// Methods.
		void AddMagicEventTrap(std::string identifier, magic_event new_magic_event);
		void CheckMagicEventTrap(std::size_t target_pin_name_hash, bool new_state);
		// Virtual methods.
		virtual void InvokeMagic(std::string const& incantation);
		virtual void UpdateMagic(void);
		virtual void ShutDownMagic(void);
		
		// Data.
		Device* m_parent_device_pointer;
		Simulation* m_top_level_sim_pointer;
		std::unordered_map<std::string, magic_event> m_magic_events;
};

#endif
