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

#include <string>					// std::string.
#include <vector>					// std::vector
#include <unordered_map>			// std::unordered_map
#include <iostream>					// std::cout, std::endl.

#include "c_core.h"					// Core simulator functionality
#include "utils.h"

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

bool IsHashInMapKeys(std::size_t key_to_find, std::unordered_map<std::size_t, pin> const& map_to_search) {
	for (const auto& entry: map_to_search) {
		if (key_to_find == entry.first) {
			return true;
		}
	}
	return false;
}

bool IsStringInVector(std::string const& string_to_find, std::vector<std::string> const& vector_to_search) {
	for (const auto& entry: vector_to_search) {
		if (string_to_find == entry) {
			return true;
		}
	}
	return false;
}

bool IsHashInVector(std::size_t hash_to_find, std::vector<std::size_t> const& vector_to_search) {
	for (const auto& entry: vector_to_search) {
		if (hash_to_find == entry) {
			return true;
		}
	}
	return false;
}

bool IsStringInMapKeys(std::string const& key_to_find, std::unordered_map<std::string, bool> const& map_to_search) {
	for (const auto& entry: map_to_search) {
		if (key_to_find == entry.first) {
			return true;
		}
	}
	return false;
}

bool IsHashInMapKeys(std::size_t key_to_find, std::unordered_map<std::size_t, bool> const& map_to_search) {
	for (const auto& entry: map_to_search) {
		if (key_to_find == entry.first) {
			return true;
		}
	}
	return false;
}

bool IsHashInMapKeys(std::size_t key_to_find, std::unordered_map<std::size_t, connection_descriptor> const& map_to_search) {
	for (const auto& entry: map_to_search) {
		if (key_to_find == entry.first) {
			return true;
		}
	}
	return false;
}

bool IsStringInMapKeys(std::string const& key_to_find, std::unordered_map<std::string, Component*> const& map_to_search) {
	for (const auto& entry: map_to_search) {
		if (key_to_find == entry.first) {
			return true;
		}
	}
	return false;
}

bool IsStringInMapKeys(std::string const& key_to_find, std::unordered_map<std::string, connection_descriptor> const& map_to_search) {
	for (const auto& entry: map_to_search) {
		if (key_to_find == entry.first) {
			return true;
		}
	}
	return false;
}

bool IsStringInMapKeys(std::string const& key_to_find, std::unordered_map<std::string, Clock*> const& map_to_search) {
	for (const auto& entry: map_to_search) {
		if (key_to_find == entry.first) {
			return true;
		}
	}
	return false;
}

bool IsHashInMapKeys(std::size_t key_to_find, std::unordered_map<std::size_t, Clock*> const& map_to_search) {
	for (const auto& entry: map_to_search) {
		if (key_to_find == entry.first) {
			return true;
		}
	}
	return false;
}

bool IsStringInMapKeys(std::string const& key_to_find, std::unordered_map<std::string, Probe*> const& map_to_search) {
	for (const auto& entry: map_to_search) {
		if (key_to_find == entry.first) {
			return true;
		}
	}
	return false;
}

bool IsStringInMapKeys(std::string const& key_to_find, std::unordered_map<std::string, MagicEngine*> const& map_to_search) {
	for (const auto& entry: map_to_search) {
		if (key_to_find == entry.first) {
			return true;
		}
	}
	return false;
}

bool IsStringInMapKeys(std::string const& key_to_find, std::unordered_map<std::string, magic_event> const& map_to_search) {
	for (const auto& entry: map_to_search) {
		if (key_to_find == entry.first) {
			return true;
		}
	}
	return false;
}

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
