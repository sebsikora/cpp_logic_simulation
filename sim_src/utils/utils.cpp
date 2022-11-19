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

#include <sstream>					// std::stringstream
#include <string>					// std::string.
#include <vector>					// std::vector
#include <unordered_map>			// std::unordered_map
#include <iostream>					// std::cout, std::endl.
#include <cmath>

#include "c_structs.hpp"
#include "utils.h"

Converter::Converter(int bit_width) {
	m_bit_width = bit_width;
	for (int i = 0; i < bit_width; i ++) {
		m_powers_of_two.push_back(pow(2, i));
	}
	m_max_int = 2 * m_powers_of_two.back();
}

void Converter::IntToStates(int int_to_convert, std::vector<bool> &state_vector) {
	for (int bit_position = 0; bit_position < m_bit_width; bit_position ++) {
		// Single & is bitwise AND.
		if ((int_to_convert >> bit_position) & 1U) {
			state_vector[bit_position] = true;
		} else {
			state_vector[bit_position] = false;
		}
	}
}

int Converter::StatesToInt(std::vector<bool> &state_vector) {
	// Will only convert places up to m_bit_width set in constructor.
	int integer_value = 0;
	for (int bit_position = 0; bit_position < m_bit_width; bit_position ++) {
		if (state_vector[bit_position]) {
			integer_value |= 1UL << bit_position;
		}
	}
	return integer_value;
}

std::vector<bool> IntToStates(int value_to_convert, int pin_count) {
	std::vector<bool> converted_value;
	for (int pin_index = 0; pin_index < pin_count; pin_index ++) {
		if ((value_to_convert & (1 << pin_index)) == (1 << pin_index)) {
			converted_value.push_back(true);
		} else {
			converted_value.push_back(false);
		}
	}
	return converted_value;
}

bool IsStringInVector(std::string const& string_to_find, std::vector<std::string> const& vector_to_search) {
	for (const auto& entry: vector_to_search) {
		if (string_to_find == entry) {
			return true;
		}
	}
	return false;
}

//~bool IsStringInMapKeys(std::string const& key_to_find, std::unordered_map<std::string, magic_event> const& map_to_search) {
	//~for (const auto& entry: map_to_search) {
		//~if (key_to_find == entry.first) {
			//~return true;
		//~}
	//~}
	//~return false;
//~}

void StaggerPrint(int level, int width) {
	std::cout << " ";
	for (int i = 0; i < level; i ++) {
		for (int j = 0; j < width; j ++) {
			std::cout << " ";
		}
	}
}

void StaggerPrint(int width) {
	std::cout << " ";
	for (int j = 0; j < width; j ++) {
		std::cout << " ";
	}
}

char BoolToChar(bool logical_state) {
	if (logical_state) {
		return 'T';
	} else {
		return 'F';
	} 
}

std::string GenerateHeader(std::string header_message) {
	std::string header = "\x1B[1m";
	int message_length = header_message.size();
	//~bool is_odd;
	if ((message_length % 2) != 0) {	// length is odd.
		header_message += " ";
		message_length += 1;
	}
	int dashes = (80 - (message_length - 2)) / 2;

	for (int i = 0; i < dashes; i ++) {
		header += "-";
	}
	header += " ";
	header += "\x1B[31m";
	header += header_message;
	header += "\x1B[0m\x1B[1m";
	header += " ";

	for (int i = 0; i < dashes; i ++) {
		header += "-";
	}
	header += "\x1B[0m";
	return header; 
}

std::string PointerToString(void* pointer){
	std::stringstream ss;
	ss << pointer;
	return ss.str();
}

std::vector<bool> IsStringInStateDescriptorVector(std::string const& string_to_find, std::vector<state_descriptor> state_descriptor_vector_to_search) {
	bool found = false;
	bool value = false;
	for (const auto& this_state_descriptor : state_descriptor_vector_to_search) {
		if (this_state_descriptor.identifier == string_to_find) {
			found = true;
			value = this_state_descriptor.state;
			break;
		}
	}
	return {found, value};
}
