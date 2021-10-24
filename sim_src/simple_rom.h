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

#ifndef LSIM_S_ROM_H
#define LSIM_S_ROM_H

#include <string>					// std::string.
#include <vector>					// std::vector

#include "c_core.h"					// Core simulator functionality

// -----------------------------------------------------------------------------------------------------------------------------------------------------
class SimpleRom : public Device {
	public:
		// Constructor.
		SimpleRom(Device* parent_device_pointer, std::string device_name, std::string data_filepath, bool monitor_on, std::vector<state_descriptor> in_pin_default_states = {});
		// Methods.
		void Build(void);
		void ConfigureMagic(Device* parent_device_pointer, std::string data_filepath);
		void ConfigureBusses(std::vector<state_descriptor> in_pin_default_states);
		// Data.
};

// -----------------------------------------------------------------------------------------------------------------------------------------------------
class SimpleRom_MagicEngine : public MagicEngine {
	public:
		// Constructor.
		SimpleRom_MagicEngine(Device* parent_device_pointer, std::string data_filepath);
		// Methods common to base MagicEngine class.
		virtual void InvokeMagic(std::string const& incantation) override;
		// Methods particular to SimpleRom_MagicEngine sub-class.
		std::vector<std::vector<bool>> Configure(std::string file_path);
		std::vector<std::string> GenerateInputs(void);
		std::vector<std::string> GenerateOutputs(void);
		void GetPinPortIndices();
		// Data particular to this sub-class.
		std::vector<int> m_powers_of_2 = {1, 2, 4, 8, 16, 32, 64, 128};
		std::string m_data_filepath;
		int m_address_bus_width;
		int m_data_bus_width;
		std::vector<std::vector<bool>> m_data;
		std::vector<int> m_address_bus_pin_port_indices;
		std::vector<int> m_data_bus_pin_port_indices;
};

#endif
