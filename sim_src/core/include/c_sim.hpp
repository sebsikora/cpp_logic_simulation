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

#ifndef LSIM_CORE_SIM_HPP
#define LSIM_CORE_SIM_HPP

#include <string>
#include <vector>
#include <thread>

#include "c_device.hpp"
#include "c_special.hpp"
#include "void_thread_pool.hpp"
#include <termios.h>				// terminal settings data structure

class Component;
class Clock;
class Probe;
class VoidThreadPool;

// Top-level Simulation Device sub-class.
class Simulation : public Device {
	public:
		Simulation(std::string const& simulation_name, SolverConfiguration solver_conf = {false, 0}, int max_propagations = 100);
		virtual ~Simulation();
		
		// Override Component virtual methods.
		void PurgeComponent(void) override;
		void SolveBackwardsFromParent(void) override {};

		// Override Device virtual methods.
		void Build(void) override;
		
		void Run(int number_of_ticks = 0, bool restart_flag = true, bool print_probes_flag = false, bool force_no_messages = false);
		void AddClock(std::string const& clock_name, std::vector<bool> const& toggle_pattern, bool monitor_on);
		void ClockConnect(std::string const& target_clock_name, std::string const& target_component_name, std::string const& target_terminal_name);
		void AddProbe(std::string const& probe_name, std::string const& target_component_full_name, std::vector<std::string> const& target_pin_names,
			std::string const& trigger_clock_name, ProbeConfiguration probe_conf = {1, 0, {'F', 'T'}}
		);
		void AddSpecialDevice(SpecialInterface* special_device);
		int GetTopLevelMaxPropagations(void);
		int GetNewCUID(void);
		Clock* GetClockPointer(std::string const& target_clock_name);
		bool IsSimulationRunning(void);
		int GetGlobalTickIndex(void);
		void StopSimulation(void);
		void CheckProbeTriggers(void);
		void LogError(std::string const& error_message);
		void LogMessage(std::string const& message);
		std::vector<std::vector<std::vector<bool>>> GetProbedStates(std::vector<std::string> const& probe_names);
		void PurgeComponentFromClocks(Component* target_component_pointer);
		void PurgeComponentFromProbes(Component* target_component_pointer);
		void PurgeComponentFromProbableComponents(Component* target_component_pointer);
		void PurgeChildProbe(std::string const& target_probe_name);
		void PurgeProbeDescriptorFromSimulation(Probe* target_probe_pointer);
		void PurgeChildClock(std::string const& target_clock_name);
		void PurgeClockDescriptorFromSimulation(Clock* target_clock_pointer);
		void PurgeGlobalComponent(std::string const& target_component_full_name);
		bool GetSearchingFlag(void);
		void SetSearchingFlag(bool value);
		void PrintErrorMessages(void);
		void PrintAndClearMessages(bool force_no_messages);
		
		VoidThreadPool* m_thread_pool_pointer = 0;
		bool m_use_threaded_solver = false;
		int m_threaded_solve_nesting_level = 0;
				
	private:
		void EnableTerminalRawIO(const bool raw_flag);
		char CheckForCharacter(void);
		
		std::vector<Probe*> m_probes;
		std::vector<Clock*> m_clocks;
		std::vector<std::string> m_error_messages;
		std::vector<std::string> m_messages;
		termios m_old_term_io_settings;
		bool m_simulation_running;
		int m_global_tick_index;
		int m_next_new_CUID;
		bool m_searching_flag = false;
		std::mutex m_sim_lock;

		std::vector<SpecialInterface*> m_special_devices;
};

#endif	// LSIM_CORE_SIM_HPP
