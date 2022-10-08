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

#ifndef LSIM_CORE_PROBE_HPP
#define LSIM_CORE_PROBE_HPP

#include <string>
#include <vector>

#include "c_structs.hpp"

class Component;
class Simulation;
class Clock;

// Probe utility class.
class Probe {
	public:
		Probe(Simulation* top_level_sim_pointer, std::string const& probe_name, Component* target_component_pointer,
			std::vector<std::string> const& target_pin_names, Clock* trigger_clock_pointer,
			probe_configuration probe_conf = {1, 0, {"F", "T"}});
		~Probe();
		
		void PreallocateSampleMemory(int number_of_ticks);
		void Sample(const int index);
		void Reset(void);
		Component* GetTargetComponentPointer(void);
		void PrintSamples(void);
		std::vector<std::vector<bool>> GetSamples(void);
		void PurgeProbe(void);
		std::string GetName(void);
		
	private:
		Simulation* m_top_level_sim_pointer;
		std::string m_name;
		std::string m_target_component_full_name;
		Component* m_target_component_pointer;
		std::vector<int> m_target_pin_indices;
		std::string m_trigger_clock_name;
		Clock* m_trigger_clock_pointer;
		int m_samples_per_row;
		int m_probe_every_n_ticks;
		std::vector<int> m_timestamps;
		std::vector<std::vector<bool>> m_samples;
		std::vector<bool> m_this_sample;
		std::vector<std::string> m_output_characters = {};
};

#endif // LSIM_CORE_PROBE_HPP
