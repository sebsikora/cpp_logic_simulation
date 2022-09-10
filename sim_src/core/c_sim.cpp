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

#include <stdexcept>				// std::out_of_range.
#include <unordered_map>			// std::unordered_map.
#include <string>					// std::string.
#include <iostream>					// std::cout, std::endl.
#include <vector>					// std::v.ctor
#include <ctime>					// time().
#include <cstdlib>					// srand().
#include <mutex>					// std::mutex, std::unique_lock.
#include <algorithm>				// std::equal.

#include <termios.h>				// terminal settings.
#include <unistd.h>					// POSIX bits.
#include <sys/ioctl.h>				// ioctl().

#include "c_structs.hpp"
#include "c_sim.hpp"
#include "c_clock.hpp"
#include "c_probe.hpp"
#include "c_m_engine.hpp"

#include "void_thread_pool.hpp"
#include "utils.h"
#include "colors.h"

Simulation::Simulation(std::string const& simulation_name, bool verbose_output_flag, solver_configuration solver_conf, int max_propagations)
 : Device(this, simulation_name, "simulation", {}, {}, false, {}, max_propagations) {
	m_next_new_CUID = 1;
	m_simulation_running = false;
	m_global_tick_index = 0;
	mg_verbose_flag = verbose_output_flag;
	mg_verbose_destructor_flag = false;					// Set to true to check destructor messages.
	m_use_threaded_solver = solver_conf.use_threaded_solver;
	m_threaded_solve_nesting_level = solver_conf.threaded_solve_nesting_level;
	m_solve_children_in_own_threads = (m_use_threaded_solver && (m_nesting_level == m_threaded_solve_nesting_level));
	std::string message = "\n" + GenerateHeader("Simulation build started.") + "\n";
	LogMessage(message);
	if (mg_verbose_flag == false) {
		std::string message = "(Simulation verbose output is off)";
		LogMessage(message);
	}
	// Start up the Simulation's solver threadpool.
	if (m_use_threaded_solver) {
		m_thread_pool_pointer = new VoidThreadPool(false);
	}
	srand(time(0));
}

Simulation::~Simulation() {
	if (m_use_threaded_solver) {
		delete m_thread_pool_pointer;
	}
	PurgeComponent();
	if (mg_verbose_destructor_flag) {
		std::cout << "Simulation dtor for " << m_full_name << " @ " << this << std::endl;
	}
}

int Simulation::GetNewCUID() {
	int new_CUID = m_next_new_CUID;
	m_next_new_CUID ++;
	return new_CUID;
}

void Simulation::EnableTerminalRawIO(const bool raw_flag) {
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

void Simulation::Run(int number_of_ticks, bool restart_flag, bool verbose_output_flag, bool print_probes_flag, bool force_no_messages) {
	bool print_errors_flag = false;
	if (m_error_messages.size() == 0) {
		bool previous_verbose_output_flag = mg_verbose_flag;
		mg_verbose_flag = verbose_output_flag;
		m_simulation_running = true;
		if (restart_flag) {
			if (!force_no_messages) {
				std::string message = GenerateHeader("Simulation started (" + std::to_string(number_of_ticks) + ").");
				std::cout << std::endl << message << std::endl;
			}
			m_global_tick_index = 0;
			for (const auto& this_clock_descriptor : m_clocks) {
				this_clock_descriptor.clock_pointer->Reset();
			}
		} else {
			if (!force_no_messages) {
				std::string message = GenerateHeader("Simulation restarted @ tick " + std::to_string(m_global_tick_index) + " (" + std::to_string(number_of_ticks) + ").");
				std::cout << std::endl << message << std::endl;
			}
		}
		// Preallocate vectors for storage of Probe samples.
		for (const auto& this_probe_descriptor : m_probes) {
			this_probe_descriptor.probe_pointer->PreallocateSampleMemory(number_of_ticks);
		}
		if ((!mg_verbose_flag) && (!force_no_messages)) {
			std::string message = std::string("\n(Simulation verbose output is off)");
			std::cout << message << std::endl;
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
			if ((mg_verbose_flag) && (!force_no_messages)){
				std::string message = GenerateHeader("Start of global tick " + std::to_string(m_global_tick_index));
				LogMessage("\n" + message);
			}
			// Advance all clocks.
			for (const auto& this_clock_descriptor : m_clocks) {
				this_clock_descriptor.clock_pointer->Tick();
			}
			// Solve top-level simulation state.
			Solve(false, m_CUID);
			// Print any messages logged this tick.
			//~if (mg_verbose_flag) {
				PrintAndClearMessages();
			//~}
			// If any errors have been reported this tick, break here and finish.
			// Asserted the __ALL_STOP__ internal input of a Device will log an error message and hence stop the simulation.
			if (m_error_messages.size() > 0) {
				print_errors_flag = true;
				m_simulation_running = false;
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
		if (!force_no_messages) {
			std::cout << std::endl << GenerateHeader("Done.") << std::endl;
		}
		// Print probe samples.
		if (print_probes_flag) {
			std::cout << std::endl << GenerateHeader("Probed values.") << std::endl << std::endl;
			for (const auto& this_probe_descriptor : m_probes) {
				this_probe_descriptor.probe_pointer->PrintSamples();
				std::cout << std::endl;
			}
			std::cout << GenerateHeader("Done.") << std::endl;
		}
		mg_verbose_flag = previous_verbose_output_flag;
	} else {
		print_errors_flag = true;
	}
	if (print_errors_flag) {
		PrintErrorMessages();
	}
}

void Simulation::AddClock(std::string const& clock_name, std::vector<bool> const& toggle_pattern, bool monitor_on) {
	bool found = false;
	for (const auto& this_clock_descriptor : m_clocks) {
		if (this_clock_descriptor.clock_name == clock_name) {
			found = true;
			break;
		}
	}
	if (!found) {
		clock_descriptor new_clock_descriptor;
		new_clock_descriptor.clock_name = clock_name;
		new_clock_descriptor.clock_pointer = new Clock(this, clock_name, toggle_pattern, monitor_on);
		m_clocks.push_back(new_clock_descriptor);
	} else {
		// Log error - Component is not on probable components list.
		std::string build_error = "Clock " + clock_name + " can not be be created as another clock by this name already exists.";
		LogError(build_error);
	}
}

void Simulation::ClockConnect(std::string const& target_clock_name, std::string const& target_component_name, std::string const& target_terminal_name) {
	Clock* target_clock_pointer = GetClockPointer(target_clock_name);
	if (target_clock_pointer != 0) {
		target_clock_pointer->Connect(target_component_name, target_terminal_name);
	} else {
		// Log error - Target clock does not exist.
		std::string build_error = "Clock " + target_clock_name + " can not be connected onward because it does not exist.";
		LogError(build_error);
	}
}

void Simulation::AddProbe(std::string const& probe_name, std::string const& target_component_full_name, std::vector<std::string> const& target_pin_names, std::string const& trigger_clock_name, probe_configuration probe_conf) {
	Component* target_component_pointer = SearchForComponentPointer(target_component_full_name);
	if (target_component_pointer != 0) {
		bool pins_exist = true;
		for (const auto& this_pin_name : target_pin_names) {
			pins_exist &= target_component_pointer->CheckIfPinExists(this_pin_name);
			// If pins_exist ends up false, at least one of the pins does not exist.
		}
		if (pins_exist) {
			Clock* trigger_clock_pointer = 0;
			bool trigger_clock_exists = false;
			for (const auto& this_clock_descriptor : m_clocks) {
				if (this_clock_descriptor.clock_name == trigger_clock_name) {
					trigger_clock_exists = true;
					trigger_clock_pointer = this_clock_descriptor.clock_pointer;
					break;
				}
			}
			if (trigger_clock_exists) {
				probe_descriptor new_probe;
				new_probe.probe_name = probe_name;
				new_probe.probe_pointer = new Probe(m_top_level_sim_pointer, probe_name, target_component_pointer, target_pin_names, trigger_clock_pointer, probe_conf);
				m_probes.push_back(new_probe);
			} else {
				// Log error - Trigger clock does not exist.
				std::string build_error = "Probe " + probe_name + " can not be added to the top-level Simulation because trigger clock " + trigger_clock_pointer->GetName() + " does not exist.";
				LogError(build_error);
			}
		} else {
			// Log error - One or more of the specified pins do not exist.
			std::string build_error = "Probe " + probe_name + " can not be added to the top-level Simulation because one or more of the specified target pins do not exist.";
			LogError(build_error);
		}
	} else {
		// Log error - Component is not on probable components list.
		std::string build_error = "Probe " + probe_name + " can not be added to the top-level Simulation because target Component " + target_component_full_name + " does not exist";
		LogError(build_error);
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
	Clock* clock_pointer = 0;
	for (const auto& this_clock_descriptor : m_clocks) {
		if (this_clock_descriptor.clock_name == target_clock_name) {
			clock_pointer = this_clock_descriptor.clock_pointer;
			break;
		}
	}
	return clock_pointer;
}

bool Simulation::IsSimulationRunning() {
	if (!m_use_threaded_solver) {
		return m_simulation_running;
	} else {
		std::unique_lock<std::mutex> lock(m_sim_lock);
		return m_simulation_running;
	}
}

void Simulation::StopSimulation() {
	if (!m_use_threaded_solver) {
		m_simulation_running = false;
	} else {
		std::unique_lock<std::mutex> lock(m_sim_lock);
		m_simulation_running = false;
	}
}

void Simulation::CheckProbeTriggers() {
	for (const auto& this_clock_descriptor : m_clocks) {
		if (this_clock_descriptor.clock_pointer->GetTickedFlag()) {
			this_clock_descriptor.clock_pointer->TriggerProbes();
		}
	}
}

void Simulation::LogError(std::string const& error_message) {
	if (!m_use_threaded_solver) {
		m_error_messages.emplace_back(error_message);
	} else {
		std::unique_lock<std::mutex> lock(m_sim_lock);
		m_error_messages.emplace_back(error_message);
	}
}

void Simulation::LogMessage(std::string const& message) {
	if (!m_use_threaded_solver) {
		m_messages.emplace_back(message);
	} else {
		std::unique_lock<std::mutex> lock(m_sim_lock);
		m_messages.emplace_back(message);
	}
}

void Simulation::PrintAndClearMessages() {
	std::unordered_map<int, std::vector<std::string>> message_collations;
	for (auto& this_message : m_messages) {
		std::string start_prefix = std::string("~S");	// Solution branch start.
		std::string end_prefix = std::string("~E");		// Solution branch end.
		std::string message_to_collate_prefix = std::string("~");		// Prefixed message.
		if (std::equal(start_prefix.begin(), start_prefix.end(), this_message.begin())) {
			// Start of messages from a new solution branch.
			this_message.erase(0, 2);
			int branch_prefix = std::stoi(this_message);
			message_collations[branch_prefix].clear();		// operator[] will create an entry via default constructor if none with key branch_prefix.
			std::cout << std::endl << "Started collating messages from branch " << branch_prefix << std::endl;
		} else if (std::equal(end_prefix.begin(), end_prefix.end(), this_message.begin())) {
			// End of this solution branch.
			this_message.erase(0, 2);
			int branch_prefix = std::stoi(this_message);
			std::cout << std::endl << "Finished collating messages from branch " << branch_prefix << std::endl;
			// Print messages from this branch to the console and then clear the dictionary entry.
			for (const auto& this_collated_message : message_collations[branch_prefix]) {
				std::cout << this_collated_message << std::endl;
			}
			message_collations.erase(branch_prefix);
		} else if (std::equal(message_to_collate_prefix.begin(), message_to_collate_prefix.end(), this_message.begin())) {
			// Message with a branch prefix.
			this_message.erase(0, 1);
			int prefix = std::stoi(this_message.substr(0, this_message.find(":", 0)));
			// operator[] will create an entry under that key if none exists, which can lead to fairly quiet
			// if not silent failure in the event that anything unexpected gets through.
			try {
				message_collations.at(prefix).push_back(this_message);
			}
			catch (const std::out_of_range& oor_error) {
				std::cerr << "Unexpected branch prefix on this message: " << this_message << std::endl;
			}
		} else {
			// No prefix at all, just print the message.
			std::cout << this_message << std::endl;
		}
	}
	m_messages.clear();
}

void Simulation::PrintErrorMessages(void) {
	if (m_error_messages.size() > 0) {
		std::cout << std::endl << GenerateHeader("Error messages.") << std::endl << std::endl;
		int index = 0;
		for (const auto& this_build_error : m_error_messages) {
			std::cout << "Error " << std::to_string(index) << " : " << this_build_error << std::endl;
			index ++;
		}
		std::cout << std::endl << GenerateHeader("Done.") << std::endl;
	}
}

std::vector<std::vector<std::vector<bool>>> Simulation::GetProbedStates(std::vector<std::string> const& probe_names) {
	std::vector<std::vector<std::vector<bool>>> probed_states;
	if (probe_names[0] == "all") {
		for (const auto& this_probe_descriptor : m_probes) {
			probed_states.emplace_back(this_probe_descriptor.probe_pointer->GetSamples());	
		}
	} else {
		for (const auto& this_probe_name : probe_names) {
			for (const auto& this_probe_descriptor : m_probes) {
				if (this_probe_descriptor.probe_name == this_probe_name) {
					probed_states.emplace_back(this_probe_descriptor.probe_pointer->GetSamples());
				}
			}
		}
	}
	return probed_states;
}

void Simulation::PurgeComponent() {
	std::string header;
	if (mg_verbose_destructor_flag) {
		header =  "Purging -> SIMULATION : " + m_full_name + " @ " + PointerToString(static_cast<void*>(this));
		std::cout << GenerateHeader(header) << std::endl;
	}
	// Set the Simulation's deletion flag to let child components know that they don't need to tidy-up
	// after themselves (remove connections from sibling components, remove themselves from Clocks,
	// Probes, etc).
	m_deletion_flag = true;
	//	Simulation has no external inputs or outputs to handle (as it is top-level).
	// 	Next we need to purge all Clocks.
	for (const auto& this_clock_descriptor : m_clocks) {
		delete this_clock_descriptor.clock_pointer;
	}
	if (m_deletion_flag) {
		// If the simulation's deletion flag is set, the Clock destructor will *not* handle deletion of Probes,
		// so we need to do it here.
		for (const auto& this_probe_descriptor : m_probes) {
			delete this_probe_descriptor.probe_pointer;
		}
	}
	if (mg_verbose_destructor_flag) {
		header =  "SIMULATION : " + m_full_name + " @ " + PointerToString(static_cast<void*>(this)) + " -> Purged.";
		std::cout << GenerateHeader(header) << std::endl;
	}
	// ~Device() will follow - this will take care of deleting all child Components.
}

void Simulation::PurgeComponentFromClocks(Component* target_component_pointer) {
	for (const auto& this_clock_descriptor : m_clocks) {
		Clock* this_clock_pointer = this_clock_descriptor.clock_pointer;
		this_clock_pointer->PurgeTargetComponentConnections(target_component_pointer);
	}
}

void Simulation::PurgeComponentFromProbes(Component* target_component_pointer) {
	// If the component has associated Probes here, nuke them.
	std::vector<Probe*> probe_pointers = {};
	for (const auto& this_probe_descriptor : m_probes) {
		if (this_probe_descriptor.probe_pointer->GetTargetComponentPointer() == target_component_pointer) {
			probe_pointers.push_back(this_probe_descriptor.probe_pointer);
		}
	}
	for (const auto& this_probe_pointer : probe_pointers) {
		delete this_probe_pointer;
	}
}

void Simulation::PurgeChildProbe(std::string const& target_probe_name) {
	// -- IF TARGET PROBE EXISTS IN M_PROBES, FETCH THE POINTER AND USE IT TO NUKE THE PROBE'S DESCRIPTOR BY CALLING PROBE.PURGEPROBE() --
	Probe* target_probe_pointer = 0;
	for (const auto& this_probe_descriptor : m_probes) {
		if (this_probe_descriptor.probe_name == target_probe_name) {
			target_probe_pointer = this_probe_descriptor.probe_pointer;
			break;
		}
	}
	if (target_probe_pointer != 0) {
		delete target_probe_pointer;
	} else {
		std::cout << "Probe " << target_probe_name << " not found." << std::endl;
	}
}

void Simulation::PurgeProbeDescriptorFromSimulation(Probe* target_probe_pointer) {
	// Rebuild m_probes omitting target Probe's probe_descriptor...
	std::vector<probe_descriptor> new_probes = {};
	for (const auto& this_probe_descriptor : m_probes) {
		if (this_probe_descriptor.probe_pointer != target_probe_pointer) {
			probe_descriptor new_probe_descriptor;
			new_probe_descriptor.probe_name = this_probe_descriptor.probe_name;
			new_probe_descriptor.probe_pointer = this_probe_descriptor.probe_pointer;
			new_probes.push_back(new_probe_descriptor);
		} else {
			if (mg_verbose_destructor_flag) {
				std::cout << "Purging " << this_probe_descriptor.probe_name << " from Simulation " << m_name << " m_probes." << std::endl;
			}
		}
	}
	m_probes = new_probes;
}

void Simulation::PurgeChildClock(std::string const& target_clock_name) {
	// -- IF TARGET CLOCK EXISTS IN M_CLOCKS, FETCH THE POINTER AND USE IT TO NUKE THE CLOCK'S DESCRIPTOR BY CALLING CLOCK.PURGECLOCK() --
	Clock* target_clock_pointer = 0;
	for (const auto& this_clock_descriptor : m_clocks) {
		if (this_clock_descriptor.clock_name == target_clock_name) {
			target_clock_pointer = this_clock_descriptor.clock_pointer;
			break;
		}
	}
	if (target_clock_pointer != 0) {
		delete target_clock_pointer;
	} else {
		std::cout << "Clock " << target_clock_name << " not found." << std::endl;
	}
}

void Simulation::PurgeClockDescriptorFromSimulation(Clock* target_clock_pointer) {
	// Rebuild m_clocks omitting target Clock's clock_descriptor...
	std::vector<clock_descriptor> new_clocks = {};
	for (const auto& this_clock_descriptor : m_clocks) {
		if (this_clock_descriptor.clock_pointer != target_clock_pointer) {
			clock_descriptor new_clock_descriptor;
			new_clock_descriptor.clock_name = this_clock_descriptor.clock_name;
			new_clock_descriptor.clock_pointer = this_clock_descriptor.clock_pointer;
			new_clocks.push_back(new_clock_descriptor);
		} else {
			if (mg_verbose_destructor_flag) {
				std::cout << "Purging " << this_clock_descriptor.clock_name << " from Simulation " << m_name << " m_clocks." << std::endl;
			}
		}
	}
	m_clocks = new_clocks;
}

void Simulation::PurgeMagicEngineDescriptorFromSimulation(magic_engine_descriptor target_descriptor) {
	std::vector<magic_engine_descriptor> new_magic_engines = {};
	for (const auto& this_magic_engine_descriptor : m_magic_engines) {
		if (this_magic_engine_descriptor != target_descriptor) {
			magic_engine_descriptor new_magic_engine_descriptor;
			new_magic_engine_descriptor.magic_engine_identifier = this_magic_engine_descriptor.magic_engine_identifier;
			new_magic_engine_descriptor.magic_engine_pointer = this_magic_engine_descriptor.magic_engine_pointer;
			new_magic_engines.push_back(new_magic_engine_descriptor);
		} else {
			if (mg_verbose_destructor_flag) {
				std::cout << "Purging " << this_magic_engine_descriptor.magic_engine_identifier << " from Simulation " << m_name << " m_magic_engines." << std::endl;
			}
		}
	}
	m_magic_engines = new_magic_engines;
}

void Simulation::PurgeGlobalComponent(std::string const& target_component_full_name) {
	Component* target_component_pointer = 0;
	target_component_pointer = m_top_level_sim_pointer->SearchForComponentPointer(target_component_full_name);
	if (target_component_pointer != 0) {
		delete target_component_pointer;
	} else {
		std::cout << "Global Component " << target_component_full_name << " not found." << std::endl;
	}
}

bool Simulation::GetSearchingFlag() {
	return m_searching_flag;
}

void Simulation::SetSearchingFlag(bool value) {
	m_searching_flag = value;
}

void Simulation::Build() {
	// Redefine for device subclass... does nothing here.
}
