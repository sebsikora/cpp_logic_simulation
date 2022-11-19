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

#ifndef LSIM_UART_H
#define LSIM_UART_H

#include <string>					// std::string.
#include <vector>					// std::vector
#include <thread>
#include <atomic>

#include "c_device.hpp"					// Core simulator functionality

// -----------------------------------------------------------------------------------------------------------------------------------------------------
class Uart : public Device {
	public:
		// Constructor.
		Uart(Device* parent_device_pointer, std::string device_name, bool monitor_on, std::vector<state_descriptor> in_pin_default_states = {});
		~Uart();
		// Methods common to base Device class.
		void Build(void) override;
		void Solve(void) override;

	private:
		// Methods.
		void Configure(std::vector<state_descriptor> in_pin_default_states);
		void CreatePty(void);
		void PtyReadRuntime(void);
		void PtyWriteByte(uint8_t byte);
		
		// Data.
		int m_master_fd;
		std::thread m_thread_pty;
		std::mutex m_mutex_master_fd;
		std::atomic_bool m_pty_available_flag = false;
		std::atomic_bool m_run_pty_thread_flag = true;
		
		std::vector<int> m_data_bus_indices;
		
		int m_data_ready_pin_index;
		int m_read_pin_index;
		int m_write_pin_index;
		int m_clk_pin_index;
};

#endif // LSIM_UART_H
