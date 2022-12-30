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

#include "c_device.hpp"
#include "c_special.hpp"

// -----------------------------------------------------------------------------------------------------------------------------------------------------
class PtyManager {
	public:
		~PtyManager();
		
		void start();
		void stop();

		bool isErrorFlagSet();
		bool rxBytesAvailable();
		uint8_t rxByte();
		void txByte(uint8_t byte);

	private:
		void createPty();
		void ptyReadRuntime();

		int m_master_fd;
		int m_pty_error_counter = 0;
		bool m_pty_error_flag = false;

		std::thread m_pty_read_thread;
		std::mutex m_mutex_master_fd;
		std::mutex m_mutex_rx_buffer;
		
		std::atomic_bool m_pty_available{false};
		std::atomic_bool m_run_threads{false};
		std::deque<uint8_t> m_rx_buffer;
};

class Uart : public Device, public SpecialInterface {
	public:
		// Constructor.
		Uart(Device* parent_device_pointer, std::string device_name, bool monitor_on, std::vector<StateDescriptor> in_pin_default_states = {});
		~Uart();
		
		// Device class method overrides.
		void Build(void) override;
		void Solve(void) override;

		// SpecialInterface class method overrides.
		void Start(void) override;
		void Update(void) override;
		void Stop(void) override;

	private:
		static const int s_data_bus_width = 8;
		
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
