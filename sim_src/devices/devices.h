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

#ifndef LSIM_DEVICES_H
#define LSIM_DEVICES_H

#include <string>					// std::string.
#include <vector>					// std::vector
#include <unordered_map>			// std::unordered_map

#include "c_core.h"					// Core simulator functionality

class JK_FF : public Device {
	public:
		JK_FF(Device* parent_device_pointer, std::string name, bool monitor_on = false, std::vector<state_descriptor> input_default_states = {});
		void Build(void);
};

class JK_FF_ASPC : public Device {
	public:
		JK_FF_ASPC(Device* parent_device_pointer, std::string name, bool monitor_on = false, std::vector<state_descriptor> input_default_states = {});
		void Build(void);
};

class Four_Bit_Counter : public Device {
	public:
		Four_Bit_Counter(Device* parent_device_pointer, std::string name, bool monitor_on = false, std::vector<state_descriptor> input_default_states = {});
		void Build(void);
};

class N_Bit_Counter : public Device {
	public:
		N_Bit_Counter(Device* parent_device_pointer, std::string name, int width, bool monitor_on = false, std::vector<state_descriptor> input_default_states = {});
		void Build(void);
		int m_width;
};

class N_Bit_Counter_AIO : public Device {
	public:
		N_Bit_Counter_AIO(Device* parent_device_pointer, std::string name, int width, bool monitor_on = false, std::vector<state_descriptor> input_default_states = {});
		//~void ConfigureOutputs(void);
		void Build(void);
		int m_width;
};

class N_Bit_Counter_ASC : public Device {
	public:
		N_Bit_Counter_ASC(Device* parent_device_pointer, std::string name, int width, bool monitor_on = false, std::vector<state_descriptor> input_default_states = {});
		void Build(void);
		int m_width;
};

class N_Bit_Counter_C_ASC : public Device {
	public:
		N_Bit_Counter_C_ASC(Device* parent_device_pointer, std::string name, int width, bool monitor_on = false, std::vector<state_descriptor> input_default_states = {});
		void Build(void);
		int m_width;
};

class One_Bit_Register : public Device {
	public:
		One_Bit_Register(Device* parent_device_pointer, std::string name, bool monitor_on = false, std::vector<state_descriptor> input_default_states = {});
		void Build(void);
};

class N_Bit_Register : public Device {
	public:
		N_Bit_Register(Device* parent_device_pointer, std::string name, int width, bool monitor_on = false, std::vector<state_descriptor> input_default_states = {});
		void Build(void);
		int m_bus_width;
};

class N_Bit_Register_ASC_AIO : public Device {
	public:
		N_Bit_Register_ASC_AIO(Device* parent_device_pointer, std::string name, int width, bool monitor_on = false, std::vector<state_descriptor> input_default_states = {});
		void Build(void);
		int m_bus_width;
};

class NxOne_Bit_Mux : public Device {
	public:
		NxOne_Bit_Mux(Device* parent_device_pointer, std::string name, int input_count, bool monitor_on = false, std::vector<state_descriptor> input_default_states = {});
		void Build(void);
		int m_input_count;
		int m_sel_bus_width;
};

class N_Bit_Decoder : public Device {
	public:
		N_Bit_Decoder(Device* parent_device_pointer, std::string name, int select_bus_width, bool monitor_on = false, std::vector<state_descriptor> input_default_states = {});
		void Build(void);
		int m_select_bus_width;
		int m_output_bus_width;
};

class NxM_Bit_Mux : public Device {
	public:
		NxM_Bit_Mux(Device* parent_device_pointer, std::string name, int bus_count, int bus_width, bool monitor_on = false, std::vector<state_descriptor> input_default_states = {});
		void Build(void);
		int m_d_bus_width;
		int m_d_bus_count;
		int m_s_bus_width;
		
};

class RLIC : public Device {
	public:
		RLIC(Device* parent_device_pointer, std::string name, bool monitor_on = false, std::vector<state_descriptor> input_default_states = {});
		void Build(void);
};

class MRLIC : public Device {
	public:
		MRLIC(Device* parent_device_pointer, std::string name, bool monitor_on = false, std::vector<state_descriptor> input_default_states = {});
		void Build(void);
};

//~// ---------------------------------------------------------------------------------------------------------------------------
//~class TestMagicDevice : public Device {
	//~public:
		//~TestMagicDevice(Device* parent_device_pointer, std::string name, bool pull_downs_on, bool monitor_on);
		//~void Build(void);
		//~void ConfigureMagic(Device* parent_device_pointer);
//~};

//~class Test_MagicEngine : public MagicEngine {
	//~public:
		//~// Constructor.
		//~Test_MagicEngine(Device* parent_device_pointer);
		//~// Methods common to base MagicEngine class.
		//~virtual void InvokeMagic(std::string const& incantation) override;
		//~// Data particular to this sub-class.
//~};
#endif
