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
#include <ctime>					// time()
#include <cstdlib>					// srand()

#include <termios.h>				// terminal settings
#include <unistd.h>					// POSIX bits.
#include <sys/ioctl.h>				// ioctl()

#include "c_core.h"					// Core simulator functionality
#include "utils.h"
#include "colors.h"

Simulation::Simulation(std::string const& simulation_name, int max_propagations, bool verbose_output_flag) : Device(static_cast<Device*>(this), simulation_name, "simulation", {}, {}, false, {}, max_propagations) {
	m_next_new_CUID = 1;
	m_simulation_running = false;
	m_global_tick_index = 0;
	mg_verbose_output_flag = verbose_output_flag;
	if (mg_verbose_output_flag == false) {
		std::cout << std::endl << "(Simulation verbose output is off)" << std::endl << std::endl;
	} else {
		std::cout << std::endl;
	}
	srand(time(0));
}

int Simulation::GetNewCUID() {
	int new_CUID = m_next_new_CUID;
	m_next_new_CUID ++;
	return new_CUID;
}

void Simulation::EnableTerminalRawIO(bool raw_flag) {
	// If raw_flag = true, we change some flags to set the current terminal to 'raw' mode, in which characters are
	// immediately made available to STDIN, such that they can be read in using getchar(). If raw_flag = false, we
	// change the flags back to their old value (captured in m_old_term_io_settings) to put the current terminal back
	// in 'buffered' mode, where characters are buffered and only made available to STDIN after a newline character.
	// Synthesised (along with the ioctl() call in CheckForCharacter() to actually check if a character is available
	// before calling getchar()) from the example at https://www.flipcode.com/archives/_kbhit_for_Linux.shtml.
	int STDIN = 0;
	if (raw_flag == true) {
		termios new_term_io_settings;
        tcgetattr(STDIN, &m_old_term_io_settings);
        new_term_io_settings = m_old_term_io_settings;
        new_term_io_settings.c_lflag &= (~ICANON & ~ECHO);
        tcsetattr(STDIN, TCSANOW, &new_term_io_settings);
        setbuf(stdin, NULL);
	} else {
		tcsetattr(STDIN, TCSANOW, &m_old_term_io_settings);
	}
}

char Simulation::CheckForCharacter() {
	int bytes_waiting, STDIN = 0;
	char key_pressed = '\0';
	ioctl(STDIN, FIONREAD, &bytes_waiting);
	if (bytes_waiting > 0) {
		key_pressed = getchar();
	}
	return key_pressed;
}

void Simulation::Run(int number_of_ticks, bool restart_flag, bool verbose_output_flag, bool print_probes_flag) {
	bool previous_verbose_output_flag = mg_verbose_output_flag;
	mg_verbose_output_flag = verbose_output_flag;
	m_simulation_running = true;
	if (restart_flag) {
		std::cout << GenerateHeader("Simulation started (" + std::to_string(number_of_ticks) + ").") << std::endl << std::endl;
		m_global_tick_index = 0;
		for (const auto& this_clock_descriptor : m_clocks) {
			this_clock_descriptor.clock_pointer->Reset();
		}
	} else {
		std::cout << GenerateHeader("Simulation restarted @ tick " + std::to_string(m_global_tick_index) + " (" + std::to_string(number_of_ticks) + ").") << std::endl << std::endl;
	}
	if (mg_verbose_output_flag == false) {
		std::cout << "(Simulation verbose output is off)" << std::endl << std::endl;
	}
	// Turn terminal to 'raw' mode (does not wait for newline before making input available to getchar()).
	// ~~~ We need to manually set it back when we are done! ~~~
	EnableTerminalRawIO(true);
	int tick_count = 0;
	int input_check_count = 0, input_check_count_limit = 1000;
	// And we're live - Spin main simulation loop.
	while (true) {
		// Update all magic device engines.
		for (const auto& this_magic_engine_descriptor : m_magic_engines) {
			this_magic_engine_descriptor.magic_engine_pointer->UpdateMagic();
		}
		if (mg_verbose_output_flag) {
			std::string msg = std::string("Start of global tick ") + std::to_string(m_global_tick_index);
			std::cout << GenerateHeader(msg) << std::endl << std::endl;
		}
		// Advance all clocks.
		for (const auto& this_clock_descriptor : m_clocks) {
			this_clock_descriptor.clock_pointer->Tick();
		}
		// Solve top-level simulation state.
		Solve();
		// If something has asserted the __ALL_STOP__ internal input somewhere, the m_sim_running flag will
		// have been de-asserted and we should break here and finish.
		if (m_simulation_running == false) {
			break;
		}
		// Every input_check_count_limit ticks check if user has pressed a key and respond.
		if (input_check_count >= input_check_count_limit) {
			char key_pressed = CheckForCharacter();
			if (key_pressed == 'q') {
				// Really we need to choose a more involved key-combination than 'q' here for the stop key...
				std::cout << std::endl << GenerateHeader("STOP KEY PRESSED") << std::endl << std::endl;
				m_simulation_running = false;
				break;
			}
			input_check_count = 0;
		} else {
			input_check_count += 1;
		}
		// number_of_ticks = 0 is continuous mode.
		m_global_tick_index += 1;
		if (number_of_ticks > 0) {
			tick_count += 1;
			if (tick_count == number_of_ticks) {
				m_simulation_running = false;
				break;
			}
		}
	}
	// Turn terminal back to 'buffered' mode (waits for newline before making input available to getchar()).
	EnableTerminalRawIO(false);	
	std::cout << GenerateHeader("Done.") << std::endl << std::endl;
	// Print probe samples.
	if (print_probes_flag) {
		std::cout << GenerateHeader("Probed values.") << std::endl << std::endl;
		for (const auto& this_probe_descriptor : m_probes) {
			this_probe_descriptor.probe_pointer->PrintSamples();
			std::cout << std::endl;
		}
		std::cout << GenerateHeader("Done.") << std::endl << std::endl;
	}
	mg_verbose_output_flag = previous_verbose_output_flag;
}

void Simulation::AddClock(std::string const& clock_name, std::vector<bool> const& toggle_pattern, bool monitor_on) {
	clock_descriptor new_clock_descriptor;
	new_clock_descriptor.clock_name = clock_name;
	new_clock_descriptor.clock_pointer = new Clock(this, clock_name, toggle_pattern, monitor_on);
	m_clocks.push_back(new_clock_descriptor);
}

void Simulation::ClockConnect(std::string const& target_clock_name, std::string const& target_component_name, std::string const& target_terminal_name) {
	GetClockPointer(target_clock_name)->Connect(target_component_name, target_terminal_name);
}

void Simulation::AddProbe(std::string const& probe_name, std::string const& target_component_full_name, std::vector<std::string> const& target_pin_names, std::string const& trigger_clock_name) {
	probe_descriptor new_probe;
	new_probe.probe_name = probe_name;
	new_probe.probe_pointer = new Probe(m_top_level_sim_pointer, probe_name, target_component_full_name, target_pin_names, trigger_clock_name);
	m_probes.push_back(new_probe);
}

void Simulation::AddToProbableDevices(std::string const& target_component_full_name, Component* target_component_pointer) {
	bool identifier_in_map = IsStringInMapKeys(target_component_full_name, m_probable_components);
	if (identifier_in_map == false) {
		m_probable_components[target_component_full_name] = target_component_pointer;
	}
}

void Simulation::AddToMagicEngines(std::string const& magic_engine_identifier, MagicEngine* magic_engine_pointer) {
	magic_engine_descriptor new_magic_engine_descriptor;
	new_magic_engine_descriptor.magic_engine_identifier = magic_engine_identifier;
	new_magic_engine_descriptor.magic_engine_pointer = magic_engine_pointer;
	m_magic_engines.push_back(new_magic_engine_descriptor);
}

int Simulation::GetTopLevelMaxPropagations() {
	return m_max_propagations;
}

Clock* Simulation::GetClockPointer(std::string const& target_clock_name) {
	Clock* clock_pointer;
	for (const auto& this_clock_descriptor : m_clocks) {
		if (this_clock_descriptor.clock_name == target_clock_name) {
			clock_pointer = this_clock_descriptor.clock_pointer;
			break;
		}
	}
	return clock_pointer;
}

Component* Simulation::GetProbableComponentPointer(std::string const& target_component_full_name) {
	return m_probable_components[target_component_full_name];
}

int Simulation::GetTopLevelComponentCount() {
	return m_components.size();
}

bool Simulation::IsSimulationRunning(void) {
	return m_simulation_running;
}

void Simulation::StopSimulation() {
	m_simulation_running = false;
}

void Simulation::ShutDown() {
	for (const auto& this_magic_engine_descriptor : m_magic_engines) {
		this_magic_engine_descriptor.magic_engine_pointer->ShutDownMagic();
	}
}

void Simulation::CheckProbeTriggers() {
	for (const auto& this_clock_descriptor : m_clocks) {
		if (this_clock_descriptor.clock_pointer->GetTickedFlag()) {
			this_clock_descriptor.clock_pointer->TriggerProbes();
		}
	}
}
