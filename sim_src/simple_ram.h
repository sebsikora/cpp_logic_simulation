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

#ifndef LSIM_S_RAM_H
#define LSIM_S_RAM_H

#include <string>					// std::string.
#include <vector>					// std::vector
#include <unordered_map>			// std::unordered_map

#include "c_core.h"					// Core simulator functionality

// -----------------------------------------------------------------------------------------------------------------------------------------------------
class SimpleRam : public Device {
	public:
		// Constructor.
		SimpleRam(Device* parent_device_pointer, std::string device_name, int address_bus_width, int data_bus_width, bool monitor_on, std::unordered_map<std::string, bool> in_pin_default_states = {});
		// Methods common to base Device class.
		void Build(void) override;
		// Methods.
		void ConfigureMagic(Device* parent_device_pointer, int address_bus_width, int data_bus_width);
		void ConfigureBusses(int address_bus_width, int data_bus_width, std::unordered_map<std::string, bool> in_pin_default_states);
		// Data.
};

// -----------------------------------------------------------------------------------------------------------------------------------------------------
class SimpleRam_MagicEngine : public MagicEngine {
	public:
		// Constructor.
		SimpleRam_MagicEngine(Device* parent_device_pointer, int address_bus_width, int data_bus_width);
		// Methods common to base MagicEngine class.
		void InvokeMagic(std::string const& incantation) override;
		void UpdateMagic(void) override;
		void ShutDownMagic(void) override;
		// Methods particular to SimpleRom_MagicEngine sub-class.
		void ZeroMemory(int address_bus_width, int data_bus_width);
		void GetPinPortIndices(int address_bus_width, int data_bus_width);
		// Data particular to this sub-class.
		std::vector<int> m_powers_of_two;
		std::vector<std::vector<bool>> m_data;
		int m_address_bus_width;
		int m_data_bus_width;
		std::vector<int> m_address_bus_pin_port_indices;
		std::vector<int> m_data_in_bus_pin_port_indices;
		std::vector<int> m_data_out_bus_pin_port_indices;
};

#endif
