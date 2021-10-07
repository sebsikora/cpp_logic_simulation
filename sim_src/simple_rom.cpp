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
#include <iostream>					// std::cout, std::endl.
#include <vector>					// std::vector
#include <unordered_map>			// std::unordered_map
#include <functional>				// std::hash
#include <cmath>					// pow()
#include <fstream>					// std::ifstream

#include "c_core.h"					// Core simulator functionality
#include "simple_rom.h"

// -----------------------------------------------------------------------------------------------------------------------------------------------------
SimpleRom::SimpleRom(Device* parent_device_pointer, std::string device_name, std::string data_filepath, bool monitor_on, std::unordered_map<std::string, bool> in_pin_default_states) 
 : Device(parent_device_pointer, device_name, "rom", {"read", "clk"}, {}, monitor_on, in_pin_default_states, 0) {
	// We call ConfigureMagic() to create the MagicEngine.
	ConfigureMagic(this, data_filepath);
	// Then we create all the address inputs and data outputs and set their default states.
	ConfigureBusses(in_pin_default_states);
	Build();
	Stabilise();
}

void SimpleRom::Build() {
	// This device does not contain any components!
	// Still need to call MakeProbable() if we want to be able to attach logic probes!
	MakeProbable();
	PrintInPinStates();
}

void SimpleRom::ConfigureMagic(Device* parent_device_pointer, std::string data_filepath) {
	m_magic_device_flag = true;
	m_magic_engine_pointer = new SimpleRom_MagicEngine(parent_device_pointer, data_filepath);
	// Create the necessary magic event trigger.
	// ROM will read data on clk transition T -> F, like a flip-flop.
	AddMagicEventTrap("clk", {true, false}, {{"read", true}}, "MEM_READ");
}

void SimpleRom::ConfigureBusses(std::unordered_map<std::string, bool> in_pin_default_states) {
	std::vector<std::string> inputs = static_cast<SimpleRom_MagicEngine*>(m_magic_engine_pointer)->GenerateInputs();
	std::vector<std::string> outputs = static_cast<SimpleRom_MagicEngine*>(m_magic_engine_pointer)->GenerateOutputs();
	CreateInPins(inputs, in_pin_default_states);
	CreateOutPins(outputs);
}

// -----------------------------------------------------------------------------------------------------------------------------------------------------
SimpleRom_MagicEngine::SimpleRom_MagicEngine(Device* parent_device_pointer, std::string data_filepath) : MagicEngine(parent_device_pointer) {
	m_data_filepath = data_filepath;
	m_data = Configure(data_filepath);
}

std::vector<std::string> SimpleRom_MagicEngine::GenerateOutputs(void) {
	std::vector<std::string> outputs_to_create;
	for (int index = 0; index < m_data_bus_width; index ++) {
		std::string output_identifier = "d_" + std::to_string(index);
		outputs_to_create.push_back(output_identifier);
		std::size_t output_identifier_hash = std::hash<std::string>{}(output_identifier);
		m_data_bus_pin_identifier_hashes.push_back(output_identifier_hash);
	}
	return outputs_to_create;
}

std::vector<std::string> SimpleRom_MagicEngine::GenerateInputs(void) {
	std::vector<std::string> inputs_to_create;
	for (int index = 0; index < m_address_bus_width; index ++) {
		std::string input_identifier = "a_" + std::to_string(index);
		inputs_to_create.push_back(input_identifier);
		std::size_t input_identifier_hash = std::hash<std::string>{}(input_identifier);
		m_address_bus_pin_identifier_hashes.push_back(input_identifier_hash);
	}
	return inputs_to_create;
}

std::vector<std::vector<bool>> SimpleRom_MagicEngine::Configure(std::string file_path) {
	// Read in data from file...
	std::vector<std::vector<bool>> padded_file_data;
	std::ifstream data_file (file_path);
	std::string file_row;
	int bus_width;
	int row_index = 0;
	if (data_file.is_open()) {
		while (std::getline(data_file, file_row)) {
			if (row_index == 0) {
				bus_width = file_row.length();
			}
			std::vector<bool> row_data;
			for (int index = 0; index < bus_width; index ++) {
				if (file_row.at(index) == '0') {
					row_data.push_back(false);
				} else {
					row_data.push_back(true);
				}
			}
			padded_file_data.push_back(row_data);
			row_index += 1;
		}
		data_file.close();
		m_data_bus_width = bus_width;
		m_address_bus_width = (int)std::ceil(std::log(row_index)/std::log(2));
		// If this address bus width lets us address more data than we have here (eg: data file has 246 rows),
		// then pad the data out with empty rows.
		int max_address = pow(2, m_address_bus_width);
		if (row_index < max_address) {
			std::vector<bool> empty_row;
			for (int i = 0; i < m_data_bus_width; i ++) {
				empty_row.push_back(false);
			}
			while (row_index < max_address) {
				padded_file_data.push_back(empty_row);
				row_index += 1;
			}
		}
	} else {
		std::cout << "Unable to open file" << std::endl;
	}
	return padded_file_data;
}

void SimpleRom_MagicEngine::InvokeMagic(std::string const& incantation) {
	if (incantation == "MEM_READ") {
		// Generate address,
		int address = 0;
		int address_pin_index = 0;
		for (const auto& pin_identifier_hash: m_address_bus_pin_identifier_hashes) {
			bool pin_state = m_parent_device_pointer->GetInPinState(pin_identifier_hash);
			if (pin_state) {
				address += pow(2, address_pin_index);
			}
			address_pin_index ++;
		}
		// Load data from member variable m_data.
		std::vector<bool> data_at_address = m_data[address];
		// Convert std::vector<bool> into states and set parent device outputs accordingly.
		int data_pin_index = 0;
		for (const auto& pin_identifier_hash: m_data_bus_pin_identifier_hashes) {
			if (m_parent_device_pointer->GetOutPinState(pin_identifier_hash) != data_at_address[data_pin_index]) {
				m_parent_device_pointer->Set(pin_identifier_hash, data_at_address[data_pin_index]);
			}
			data_pin_index ++;
		}
	} else {
		std::cout << "The incantation appears to do nothing...." << std::endl;
	}
}
