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

#ifndef LSIM_CORE_CLOCK_HPP
#define LSIM_CORE_CLOCK_HPP

#include <string>
#include <vector>

#include "c_structs.hpp"

class Component;
class Simulation;
class Probe;

// Clock utility class.
class Clock {
	public:
		Clock(Simulation* top_level_sim_pointer, std::string const& clock_name, std::vector<bool> toggle_pattern, bool monitor_on);
		~Clock();
		
		std::string GetName(void);
		void Connect(std::string const& target_component_name, std::string const& target_pin_name);
		void Propagate(void);
		void Tick(void);
		void Reset(void);
		void AddToProbeList(std::string const& probe_identifier, Probe* probe_pointer);
		void TriggerProbes(void);
		bool GetTickedFlag(void);
		void PurgeTargetComponentConnections(Component* target_component_pointer);
		void PurgeClock(void);
		void PurgeProbeDescriptorFromClock(Probe* target_probe_pointer);

	private:
		Simulation* m_top_level_sim_pointer;
		std::string m_name;
		std::vector<bool> m_toggle_pattern;
		bool m_monitor_on;
		std::vector<connection_descriptor> m_connections;
		bool m_out_pin_state;
		std::vector<bool> m_state_history;
		size_t m_index;
		size_t m_sub_index;
		std::vector<probe_descriptor> m_probes;
		bool m_ticked_flag;
};

#endif	// LSIM_CORE_CLOCK_HPP
