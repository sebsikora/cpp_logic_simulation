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

#ifndef LSIM_RAM_H
#define LSIM_RAM_H

#include <string>					// std::string.
#include <vector>					// std::vector

#include "c_device.hpp"					// Core simulator functionality

// -----------------------------------------------------------------------------------------------------------------------------------------------------
class Ram : public Device {
	public:
		// Constructor.
		Ram(Device* parent_device_pointer, std::string device_name, int address_bus_width, int data_bus_width, bool monitor_on, std::vector<state_descriptor> in_pin_default_states = {});
		~Ram();
		// Methods common to base Device class.
		void Build(void) override;
		void Solve(void) override;

	private:
		// Methods.
		void Configure(int address_bus_width, int data_bus_width, std::vector<state_descriptor> in_pin_default_states);

		// Data.
		std::vector<unsigned long> m_data;
		
		std::vector<int> m_address_bus_indices;
		std::vector<int> m_data_bus_in_indices;
		std::vector<int> m_data_bus_out_indices;
		
		int m_read_pin_index;
		int m_write_pin_index;
		int m_clk_pin_index;
};

#endif // LSIM_RAM_H
