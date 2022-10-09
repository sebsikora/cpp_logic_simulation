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

#include "c_device.hpp"					// Core simulator functionality

// -----------------------------------------------------------------------------------------------------------------------------------------------------
class SimpleRamRedux : public Device {
	public:
		// Constructor.
		SimpleRamRedux(Device* parent_device_pointer, std::string device_name, int address_bus_width, int data_bus_width, bool monitor_on, std::vector<state_descriptor> in_pin_default_states = {});
		~SimpleRamRedux();
		// Methods common to base Device class.
		void Build(void) override;
		void Solve(void) override;

	private:
		// Methods.
		void Configure(int address_bus_width, int data_bus_width, std::vector<state_descriptor> in_pin_default_states);

		// Data.
		std::vector<unsigned int> m_data;
		
		int m_address_bus_start_index;
		int m_address_bus_end_index;
		int m_data_bus_in_start_index;
		int m_data_bus_in_end_index;
		int m_data_bus_out_start_index;
		int m_data_bus_out_end_index;
		
		int m_read_pin_index;
		int m_write_pin_index;
		int m_clk_pin_index;
};

// -----------------------------------------------------------------------------------------------------------------------------------------------------
//~class SimpleRam_MagicEngine : public MagicEngine {
	//~public:
		//~// Constructor.
		//~SimpleRam_MagicEngine(Device* parent_device_pointer, int address_bus_width, int data_bus_width);
		//~~SimpleRam_MagicEngine();
		//~// Methods common to base MagicEngine class.
		//~void InvokeMagic(int incantation) override;
		//~void UpdateMagic(void) override;
		//~void ShutDownMagic(void) override;
		//~// Methods particular to SimpleRom_MagicEngine sub-class.
		//~void ZeroMemory(int address_bus_width, int data_bus_width);
		//~void GetPinPortIndices(int address_bus_width, int data_bus_width);
		//~// Data particular to this sub-class.
		//~std::vector<int> m_powers_of_two;
		//~std::vector<size_t> m_data;
		//~int m_address_bus_width;
		//~int m_data_bus_width;
		//~std::vector<int> m_address_bus_pin_port_indices;
		//~std::vector<int> m_data_in_bus_pin_port_indices;
		//~std::vector<int> m_data_out_bus_pin_port_indices;
//~};

#endif
