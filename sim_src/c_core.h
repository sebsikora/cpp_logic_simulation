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
#ifndef LSIM_CORE_H
#define LSIM_CORE_H

// Includes for this header.
#include <string>					// std::string.
#include <vector>					// std::vector
#include <unordered_map>			// std::unordered_map
#include <termios.h>				// terminal settings data structure

// Forward declarations for struct and class definitions below.
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
	std::string magic_engine_identifier;
	MagicEngine* magic_engine_pointer;
};

struct magic_event_co_condition {
	int pin_port_index; 
	bool pin_state;
};

struct human_writable_magic_event_co_condition {
	std::string pin_name;
	bool pin_state;
};

// This typedef defines the type 'pointer to a Gate class member function that takes a vector of pin
// structs as arguments and returns an output bool. This allows us to *dramatically* simplify the code
// for declaring such function pointers and member functions that take them as arguments and/or return them.
typedef bool (Gate::*operator_pointer)(std::vector<pin> const&);

// Class prototypes.
// Base Component class.
class Component {
	public:
		// Component class constructor.
		Component() {}

		// Component class virtual methods.
		virtual void Connect(std::vector<std::string> connection_parameters) = 0;
		virtual void Set(int pin_port_index, bool state_to_set) = 0;
		virtual void Propagate(void) = 0;
		virtual void MakeProbable(void) = 0;
		virtual void PrintPinStates(int max_levels) = 0;
		virtual void ReportUnConnectedPins(void) = 0;

		// Component class methods.
		std::string GetName(void);
		bool GetDeviceFlag(void);
		std::string GetFullName(void);
		std::string GetComponentType(void);
		Simulation* GetTopLevelSimPointer(void);
		bool GetPinState(int pin_port_index);
		std::string GetPinName(int pin_port_index);
		std::vector<std::string> GetSortedInPinNames(void);
		std::vector<std::string> GetSortedOutPinNames(void);
		int GetPinDirection(int pin_port_index);
		int GetPinDirection(std::string const& pin_name);
		int GetPinPortIndex(std::string const& pin_name);
		bool CheckIfPinExists(std::string const& target_pin_name);
		std::vector<bool> CheckIfPinDriven(int pin_port_index);
		void SetPinDrivenFlag(int pin_port_index, bool drive_mode, bool state_to_set);
		void PrintInPinStates(void);
		void PrintOutPinStates(void);
		
		// Component class data.
		static bool mg_verbose_output_flag;
		bool m_monitor_on;
		int m_nesting_level;
		bool m_device_flag;
		std::string m_name;
		std::string m_full_name;
		int m_CUID;
		int m_local_component_index = 0;
		std::string m_component_type;
		Simulation* m_top_level_sim_pointer;
		Device* m_parent_device_pointer;
		std::vector<pin> m_pins;
};

// Logic Gate Component sub-class.
class Gate : public Component {
	public:
		// Gate class constructor.
		Gate(Device* parent_device_pointer, std::string const& gate_name, std::string const& gate_type,
		std::vector<std::string> in_pin_names = {}, bool monitor_on = false
		);
		
		// Override Component virtual methods.
		void Connect(std::vector<std::string> connection_parameters) override;
		void Set(int pin_port_index, bool state_to_set) override;
		void Propagate(void) override;
		void MakeProbable(void) override;
		void PrintPinStates(int max_levels) override;
		void ReportUnConnectedPins(void) override;
		
		// Gate class methods.
		void Initialise(void);
		void Evaluate(void);
		Component* GetSiblingComponentPointer(std::string const& target_sibling_component_name);
		operator_pointer GetOperatorPointer(std::string const& operator_name);
		bool OperatorAnd(std::vector<pin> const& pins);
		bool OperatorNand(std::vector<pin> const& pins);
		bool OperatorOr(std::vector<pin> const& pins);
		bool OperatorNor(std::vector<pin> const& pins);
		bool OperatorNot(std::vector<pin> const& pins);
		
		// Gate class data.
		int m_out_pin_port_index;
		operator_pointer m_operator_function_pointer;
		std::vector<connection_descriptor> m_connections;
};

// Compound-logic Device Component sub-class. 
class Device : public Component {
	public:
		// Device class constructor.
		Device(Device* parent_device_pointer, std::string const& device_name, std::string const& device_type, std::vector<std::string> in_pin_names,
		std::vector<std::string> out_pin_names, bool monitor_on = false, std::unordered_map<std::string, bool> const& in_pin_default_states = {},
		int max_propagations = 0
		);
		
		// Override Component virtual methods.
		void Connect(std::vector<std::string> connection_parameters) override;
		void Set(int pin_port_index, bool state_to_set) override;
		void Propagate(void) override;
		void MakeProbable(void) override;
		void PrintPinStates(int max_levels) override;
		void ReportUnConnectedPins(void) override;
		
		// Device class methods.
		virtual void Build(void);
		void CreateInPins(std::vector<std::string> const& pin_names, std::unordered_map<std::string, bool> pin_default_states);
		void CreateOutPins(std::vector<std::string> const& pin_names);
		void AddComponent(Component* new_component_pointer);
		void AddGate(std::string const& component_name, std::string const& component_type, std::vector<std::string> const& in_pin_names, bool monitor_on);
		void AddGate(std::string const& component_name, std::string const& component_type, bool monitor_on);
		void AddGate(std::string const& component_name, std::string const& component_type);
		void AddMagicEventTrap(std::string const& target_pin_name, std::vector<bool> const& state_change,
			std::vector<human_writable_magic_event_co_condition> const& hw_co_conditions, std::string const& incantation
		);
		void ChildConnect(std::string const& target_child_component_name, std::vector<std::string> const& connection_parameters);
		void ChildSet(std::string const& target_child_component_name, std::string const& target_pin_name, bool logical_state);
		void ChildPrintPinStates(std::string const& target_child_component_name, int max_levels);
		void ChildPrintInPinStates(std::string const& target_child_component_name);
		void ChildPrintOutPinStates(std::string const& target_child_component_name);
		void ChildMakeProbable(std::string const& target_child_component_name);
		void ChildMarkOutputNotConnected(std::string const& target_child_component_name, std::string const& target_out_pin_name);
		void Connect(std::string const& origin_pin_name, std::string const& target_component_name, std::string const& target_pin_name = "input");
		void Stabilise(void);
		bool Solve(void);
		void SubTick(int index);
		Component* GetChildComponentPointer(std::string const& target_child_component_name);
		int GetNestingLevel(void);
		int GetNewLocalComponentIndex(void);
		bool CheckIfQueuedToPropagateThisTick(int propagation_identifier);
		void AddToPropagateNextTick(int propagation_identifier);
		void PrintInternalPinStates(int max_levels);
		std::vector<std::string> GetHiddenInPins(void);
		void MarkInnerTerminalsDisconnected(void);
		bool CheckAndClearSolutionFlag(void);
		
		// Device class data.
		int m_max_propagations;
		std::vector<component_descriptor> m_components;
		std::vector<int> m_devices;
		std::vector<int> m_propagate_next_tick;
		std::vector<int> m_propagate_this_tick;
		std::vector<int> m_still_to_propagate;
		bool m_buffered_propagation = false;
		bool m_solve_this_tick_flag = false;
		std::vector<std::vector<connection_descriptor>> m_ports; 			// Maps in- and out-pins to connection descriptors.
		bool m_magic_device_flag = false;
		MagicEngine* m_magic_engine_pointer;
		const std::vector<std::string> m_hidden_in_pins = {"true", "false"};
		const std::vector<std::string> m_hidden_out_pins = {"all_stop"};
};

// Top-level Simulation Device sub-class.
class Simulation : public Device {
	public:
		// Simulation class constructor.
		Simulation(std::string const& simulation_name, int max_propagations = 10, bool verbose_output_flag = false);
		
		// Simulation class methods.
		void Run(int number_of_ticks = 0, bool restart_flag = true, bool verbose_debug_flag = false, bool print_probes_flag = false);
		void AddClock(std::string const& clock_name, std::vector<bool> const& toggle_pattern, bool monitor_on);
		void ClockConnect(std::string const& target_clock_name, std::string const& target_component_name, std::string const& target_terminal_name);
		void AddProbe(std::string const& probe_name, std::string const& target_component_full_name, std::vector<std::string> const& target_pin_names,
			std::string const& trigger_clock_name
		);
		void AddToProbableComponents(Component* target_component_pointer);
		void AddToMagicEngines(std::string const& magic_engine_identifier, MagicEngine* magic_engine_pointer);
		int GetTopLevelMaxPropagations(void);
		int GetNewCUID(void);
		Clock* GetClockPointer(std::string const& target_clock_name);
		Component* GetProbableComponentPointer(std::string const& target_component_full_name);
		int GetTopLevelComponentCount(void);
		void EnableTerminalRawIO(bool raw_flag);
		char CheckForCharacter(void);
		bool IsSimulationRunning(void);
		void StopSimulation(void);
		void ShutDown(void);
		void CheckProbeTriggers(void);
		void LogBuildError(std::string const& build_error);
		void PrintBuildErrors(void);
		std::vector<std::vector<std::vector<bool>>> GetProbedStates(std::vector<std::string> const& probe_names);
		
		// Simulation class data.
		std::vector<Component*> m_probable_components;
		std::vector<probe_descriptor> m_probes;
		std::vector<clock_descriptor> m_clocks;
		std::vector<magic_engine_descriptor> m_magic_engines;
		std::vector<std::string> m_build_errors;
		termios m_old_term_io_settings;
		bool m_simulation_running;
		int m_global_tick_index;
		int m_next_new_CUID;
};

// Clock utility class.
class Clock {
	public:
		// Clock class constructor.
		Clock(Simulation* top_level_sim_pointer, std::string const& clock_name, std::vector<bool> toggle_pattern, bool monitor_on);
		
		// Clock class methods.
		void Connect(std::string const& target_component_name, std::string const& target_pin_name);
		void Propagate(void);
		void Tick(void);
		void Reset(void);
		void AddToProbeList(std::string const& probe_identifier, Probe* probe_pointer);
		void TriggerProbes(void);
		bool GetTickedFlag(void);
		std::string GetName(void);
		
		// Clock class data.
		Simulation* m_top_level_sim_pointer;
		std::string m_name;
		std::vector<bool> m_toggle_pattern;
		bool m_monitor_on;
		std::vector<connection_descriptor> m_connections;
		bool m_out_pin_state;
		std::vector<bool> m_state_history;
		int m_index;
		int m_sub_index;
		std::vector<probe_descriptor> m_probes;
		bool m_ticked_flag;
};

// Probe utility class.
class Probe {
	public:
		// Probe class constructor.
		Probe(Simulation* top_level_sim_pointer, std::string const& probe_name, Component* target_component_pointer,
			std::vector<std::string> const& target_pin_names, Clock* trigger_clock_pointer
		);
		
		// Probe class methods.
		void Sample(int index);
		void Reset(void);
		void PreallocateSampleMemory(int number_of_ticks);
		void PrintSamples(void);
		std::vector<std::vector<bool>> GetSamples(void);
		
		// Probe class data.
		Simulation* m_top_level_sim_pointer;
		std::string m_name;
		std::string m_target_component_full_name;
		Component* m_target_component_pointer;
		std::vector<int> m_target_pin_indices;
		std::string m_trigger_clock_name;
		Clock* m_trigger_clock_pointer;
		std::vector<int> m_timestamps;
		std::vector<std::vector<bool>> m_samples;
		std::vector<bool> m_this_sample;
};

struct magic_event {
	int target_pin_port_index;
	std::vector<bool> state_change;
	std::vector<magic_event_co_condition> co_conditions;
	std::string incantation;
};

class MagicEngine {
	public:
		// Constructor.
		MagicEngine(Device* parent_device_pointer);
		// Methods.
		void AddMagicEventTrap(std::string const& identifier, magic_event new_magic_event);
		void CheckMagicEventTrap(int target_pin_port_index, bool new_state);
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
