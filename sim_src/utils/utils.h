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

#ifndef LSIM_UTILS_H
#define LSIM_UTILS_H

#include <string>					// std::string
#include <vector>					// std::vector
#include <unordered_map>			// std::unordered_map

#include "c_structs.hpp"

std::vector<bool> IntToStates(int value_to_convert, int pin_count);
void StaggerPrint(int level, int width);
void StaggerPrint(int width);
std::string GenerateHeader(std::string header_message);
char BoolToChar(bool logical_state);
std::string PointerToString(void* pointer);

class Converter {
	public:
		Converter(int bit_width);
		~Converter() {}
		
		void IntToStates(int int_to_convert, std::vector<bool> &state_vector);
		int StatesToInt(std::vector<bool> &state_vector);
		
		std::vector<int> m_powers_of_two;
		int m_bit_width;
		int m_max_int;
};

#endif
