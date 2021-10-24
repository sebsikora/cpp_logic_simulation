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
#include <ctime>					// time()
#include <cstdlib>					// srand()

#include <termios.h>				// terminal settings
#include <unistd.h>					// POSIX bits.
#include <sys/ioctl.h>				// ioctl()

#include "c_core.h"					// Core simulator functionality
#include "utils.h"
#include "colors.h"

Simulation::Simulation(std::string const& simulation_name, int max_propagations, bool verbose_output_flag)
 : Device(this, simulation_name, "simulation", {}, {}, false, {}, max_propagations
	) {
	m_next_new_CUID = 1;
	m_simulation_running = false;
	m_global_tick_index = 0;
	mg_verbose_output_flag = verbose_output_flag;
	std::cout << GenerateHeader("Simulation build started.") << std::endl << std::endl;
	if (mg_verbose_output_flag == false) {
		std::cout << "(Simulation verbose output is off)" << std::endl << std::endl;
	} else {
		std::cout << std::endl;
	}
	srand(time(0));
}

Simulation::~Simulation() {
	PurgeComponent();
	if (mg_verbose_output_flag) {
		std::cout << "Simulation dtor for " << m_full_name << " @ " << this << std::endl;
	}
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
	if (m_build_errors.size() == 0) {
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
		// Preallocate vectors for storage of Probe samples.
		for (const auto& this_probe_descriptor : m_probes) {
			this_probe_descriptor.probe_pointer->PreallocateSampleMemory(number_of_ticks);
		}
		if (mg_verbose_output_flag == false) {
			std::cout << "(Simulation verbose output is off)" << std::endl;
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
				std::cout << GenerateHeader(msg) << std::endl;
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
		std::cout << std::endl << GenerateHeader("Done.") << std::endl << std::endl;
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
	} else {
		PrintBuildErrors();
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
		LogBuildError(build_error);
	}
}

void Simulation::ClockConnect(std::string const& target_clock_name, std::string const& target_component_name, std::string const& target_terminal_name) {
	Clock* target_clock_pointer = GetClockPointer(target_clock_name);
	if (target_clock_pointer != 0) {
		target_clock_pointer->Connect(target_component_name, target_terminal_name);
	} else {
		// Log error - Target clock does not exist.
		std::string build_error = "Clock " + target_clock_name + " can not be connected onward because it does not exist.";
		LogBuildError(build_error);
	}
}

void Simulation::AddProbe(std::string const& probe_name, std::string const& target_component_full_name, std::vector<std::string> const& target_pin_names, std::string const& trigger_clock_name) {
	Component* target_component_pointer = GetProbableComponentPointer(target_component_full_name);
	if (target_component_pointer != 0) {
		bool pins_exist = true;
		for (const auto& this_pin_name : target_pin_names) {
			pins_exist &= target_component_pointer->CheckIfPinExists(this_pin_name);
			// If pins_exist ends up false, at least one of the pins does not exist.
		}
		if (pins_exist) {
			Clock* trigger_clock_pointer;
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
				new_probe.probe_pointer = new Probe(m_top_level_sim_pointer, probe_name, target_component_pointer, target_pin_names, trigger_clock_pointer);
				m_probes.push_back(new_probe);
			} else {
				// Log error - Trigger clock does not exist.
				std::string build_error = "Probe " + probe_name + " can not be added to the top-level Simulation because trigger clock " + trigger_clock_pointer->GetName() + " does not exist.";
				LogBuildError(build_error);
			}
		} else {
			// Log error - One or more of the specified pins do not exist.
			std::string build_error = "Probe " + probe_name + " can not be added to the top-level Simulation because one or more of the specified target pins do not exist.";
			LogBuildError(build_error);
		}
	} else {
		// Log error - Component is not on probable components list.
		std::string build_error = "Probe " + probe_name + " can not be added to the top-level Simulation because target Component " + target_component_full_name + " is not on the top-level Simulation's probable Component list.";
		LogBuildError(build_error);
	}
}

void Simulation::AddToProbableComponents(Component* target_component_pointer) {
	bool found = false;
	for (const auto& this_component_pointer : m_probable_components) {
		if (this_component_pointer == target_component_pointer) {
			found = true;
			break;
		}
	}
	if (!found) {
		m_probable_components.emplace_back(target_component_pointer);
	} else {
		// Log build error here.		-- This connection already exists.
		std::string build_error = "Component " + target_component_pointer->GetFullName() + " can not be added to the probable Components list because it has already been added.";
		LogBuildError(build_error);
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

Component* Simulation::GetProbableComponentPointer(std::string const& target_component_full_name) {
	Component* target_component_pointer = 0;
	for (const auto& this_component_pointer : m_probable_components) {
		if (this_component_pointer->GetFullName() == target_component_full_name) {
			target_component_pointer = this_component_pointer;
			break;
		}
	}
	return target_component_pointer;
}

int Simulation::GetTopLevelComponentCount() {
	return m_components.size();
}

bool Simulation::IsSimulationRunning() {
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

void Simulation::LogBuildError(std::string const& build_error) {
	m_build_errors.emplace_back(build_error);
}

void Simulation::PrintBuildErrors(void) {
	std::cout << GenerateHeader("Build Errors.") << std::endl << std::endl;
	int index = 0;
	for (const auto& this_build_error : m_build_errors) {
		std::cout << "Error " << std::to_string(index) << " : " << this_build_error << std::endl;
		index ++;
	}
	std::cout << std::endl << GenerateHeader("Done.") << std::endl << std::endl;
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
	if (mg_verbose_output_flag) {
		header =  "Purging -> SIMULATION : " + m_full_name + " @ " + PointerToString(static_cast<void*>(this));
		std::cout << GenerateHeader(header) << std::endl;
	}
	// Need to Purge all child Components and delete.
	// Can't blast away at our m_components as we iterate over it, so we will make a copy on the stack and iterate over that.
	
	{	// We will do it inside a control block, to make sure that m_components_copy (with it's pointers to nowhere once we
		// nuke it's contents) goes out of scope asap.
		std::vector<component_descriptor> m_components_copy;
		for (const auto& this_component_descriptor : m_components) {
			component_descriptor new_component_descriptor;
			new_component_descriptor.component_name = this_component_descriptor.component_name;
			new_component_descriptor.component_full_name = this_component_descriptor.component_full_name;
			new_component_descriptor.component_pointer = this_component_descriptor.component_pointer;
			m_components_copy.push_back(new_component_descriptor);
		}
		
		// Now we can iterate over m_components_copy and blast away at m_components.
		for (const auto& copied_component_descriptor : m_components_copy) {
			delete copied_component_descriptor.component_pointer;
		}
	}
	//	Simulation has no external inputs or outputs to handle (as it is top-level).
	// 	Next we need to purge all Clocks.
	for (const auto& this_clock_descriptor : m_clocks) {
		delete this_clock_descriptor.clock_pointer;
	}
	if (mg_verbose_output_flag) {
		header =  "SIMULATION : " + m_full_name + " @ " + PointerToString(static_cast<void*>(this)) + " -> Purged.";
		std::cout << GenerateHeader(header) << std::endl;
	}
	// - It should now be safe to delete this object -
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

void Simulation::PurgeComponentFromProbableComponents(Component* target_component_pointer) {
	std::vector<Component*> new_probable_components = {};
	for (const auto& this_probable_component_pointer : m_probable_components) {
		if (this_probable_component_pointer != target_component_pointer) {
			new_probable_components.push_back(this_probable_component_pointer);
		} else {
			if (mg_verbose_output_flag) {
				std::cout << "Purging " << target_component_pointer->GetFullName() << " from Simulation m_probable_components." << std::endl;
			}
		}
	}
	m_probable_components = new_probable_components;
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
			if (mg_verbose_output_flag) {
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
			if (mg_verbose_output_flag) {
				std::cout << "Purging " << this_clock_descriptor.clock_name << " from Simulation " << m_name << " m_clocks." << std::endl;
			}
		}
	}
	m_clocks = new_clocks;
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
