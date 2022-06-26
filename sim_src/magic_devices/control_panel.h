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

#ifndef LSIM_C_PANEL_H
#define LSIM_C_PANEL_H

#include <string>					// std::string.
#include <vector>					// std::vector
#include <deque>					// std::deque

#include "c_core.h"					// Core simulator functionality

// -----------------------------------------------------------------------------------------------------------------------------------------------------
class ControlPanel : public Device {
	public:
		// Constructor.
		ControlPanel(Device* parent_device_pointer, std::string const& device_name, std::string const& panel_recipe, bool monitor_on, std::vector<state_descriptor> in_pin_default_states = {});
		~ControlPanel();
		// Methods common to base Device class.
		void Build(void) override;
		// Methods.
		void ConfigureMagic(Device* parent_device_pointer, std::string name);
		// Data.
		std::string m_panel_recipe = "";
};

// -----------------------------------------------------------------------------------------------------------------------------------------------------
class ControlPanel_MagicEngine : public MagicEngine {
	public:
		// Constructor.
		ControlPanel_MagicEngine(Device* parent_device_pointer, std::string name, std::string panel_recipe);
		~ControlPanel_MagicEngine();
		// Methods common to base MagicEngine class.
		void InvokeMagic(int incantation) override;
		void UpdateMagic(void) override;
		void ShutDownMagic(void) override;
		// Methods particular to SimpleRom_MagicEngine sub-class.
		void GetPinPortIndices(int data_bus_width);
		pid_t StartClient(const char *fifo_dat_s_m_identifier, const char *fifo_dat_m_s_identifier);
		int ConfigureFIFO(const char *fifo_dat_s_m_identifier, const char *fifo_dat_m_s_identifier, int *fifo_dat_s_to_m, int *fifo_dat_m_to_s);
		void SendMessage(const char* message, int fifo);
		void SendCharacter(char character, int fifo);
		void Finish(pid_t client_pid, const char *fifo_dat_s_m_identifier, const char *fifo_dat_m_s_identifier, int *fifo_dat_slave_to_master, int *fifo_dat_master_to_slave);
		
		// Data particular to this sub-class.
		
		std::string m_fifo_dat_s_m_ident_string;
		std::string m_fifo_dat_m_s_ident_string;
		const char *m_fifo_dat_s_m_ident_carray;
		const char *m_fifo_dat_m_s_ident_carray;
		int m_fifo_dat_slave_to_master;
		int m_fifo_dat_master_to_slave;
		pid_t m_client_pid;
		std::deque<char> m_data_in_char_buffer;			// Use a deque<> rather than a vector<> as deque<> has efficient insert at start.
		
};

#endif
