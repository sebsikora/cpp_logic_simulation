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
#include <deque>

#include "c_device.hpp"					// Core simulator functionality
#include "c_special.hpp"

// -----------------------------------------------------------------------------------------------------------------------------------------------------
class PtyManager {
	public:
		PtyManager();
		~PtyManager();

		void stop();

		bool uartAvailable();
		bool rxBytesAvailable();
		uint8_t rxByte();
		void txByte(uint8_t byte);

	private:
		void CreatePty(void);
		void PtyReadRuntime(void);

		int m_master_fd;

		std::thread m_thread_pty;
		std::mutex m_mutex_master_fd;
		std::mutex m_mutex_rx_buffer;

		std::atomic_bool m_pty_available_flag;
		std::atomic_bool m_run_pty_thread_flag;
		std::deque<uint8_t> m_rx_buffer;
};

class Uart : public Device, public SpecialInterface {
	public:
		// Constructor.
		Uart(Device* parent_device_pointer, std::string device_name, bool monitor_on, std::vector<StateDescriptor> in_pin_default_states = {});
		~Uart();
		// Methods common to base Device class.
		void Build(void) override;
		void Solve(void) override;

		void Update(void) override;

	private:
		// Methods.
		void Configure(std::vector<StateDescriptor> in_pin_default_states);

		PtyManager m_ptyManager;
		
		std::vector<int> m_data_bus_in_indices;
		std::vector<int> m_data_bus_out_indices;
		
		int m_data_ready_pin_index;
		int m_read_pin_index;
		int m_write_pin_index;
		int m_clk_pin_index;
};

#endif // LSIM_UART_H
