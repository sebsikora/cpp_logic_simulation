#include <chrono>
#include <cmath>
#include <cstdlib>

#include "c_core.h"			// Core simulator functionality
#include "simple_ram.h"		// SimpleRam MagicDevice
#include "utils.h"

int main () {
	// Verbosity flags. Set verbose & monitor_on equal to true to display verbose simulation output in the console.
	bool verbose = false;
	bool monitor_on = false;
	//~bool print_probe_samples = false;
	
	// Instantiate the top-level Device (the Simulation).
	Simulation* sim = new Simulation("test_sim", 10, verbose);
	
	// Add the SimpleRam MagicDevice and call stabilise.
	int data_bus_width = 16;
	int address_bus_width = 16;
	int number_of_runs = 10;
	
	sim->AddComponent(new SimpleRam(sim, "test_ram", address_bus_width, data_bus_width, monitor_on, {{"write", true}, {"read", false}}));
	sim->Stabilise();
	
	// Add a Clock and connect it to the clk input on the register.
	// The Clock output will be a repeating pattern of false, true, false, true, etc, starting on false on the first tick.
	sim->AddClock("clock_0", {false, true}, monitor_on);
	sim->ClockConnect("clock_0", "test_ram", "clk");
	
	std::vector<bool> address_pin_states = {};
	std::vector<std::string> address_pin_names = {};
	for (int i = 0; i < address_bus_width; i ++) {
		std::string this_pin_name = "a_" + std::to_string(i);
		address_pin_states.push_back(false);
		address_pin_names.push_back(this_pin_name);
	}
	Converter address_converter(address_bus_width);
	
	std::vector<bool> data_pin_states = {};
	std::vector<std::string> data_in_pin_names = {};
	std::vector<std::string> data_out_pin_names = {};
	for (int i = 0; i < data_bus_width; i ++) {
		std::string this_in_pin_name = "d_in_" + std::to_string(i);
		std::string this_out_pin_name = "d_out_" + std::to_string(i);
		data_pin_states.push_back(false);
		data_in_pin_names.push_back(this_in_pin_name);
		data_out_pin_names.push_back(this_out_pin_name);
	}
	Converter data_converter(data_bus_width);
	
	int address_max = pow(2, address_bus_width);
	int data_max = pow(2, data_bus_width);
	
	std::vector<long long> run_times = {};
	
	for (int run_index = 0; run_index < number_of_runs; run_index ++) {
		// Make sure the Device is in 'write' mode.
		sim->ChildSet("test_ram", "write", true);
		sim->ChildSet("test_ram", "read", false);
		
		// Generate the test data.
		std::vector<int> write_data = {};
		for (int address = 0; address < address_max; address ++) {
			write_data.push_back(rand() % data_max);
		}
		
		// Iterate over the whole address range writing the test data.
		long long sub_total_1 = 0;
		for (int address = 0; address < address_max; address ++) {
			// Set address pins.
			address_converter.IntToStates(address, address_pin_states);
			for (int pin_index = 0; pin_index < address_bus_width; pin_index ++) {
				sim->ChildSet("test_ram", address_pin_names[pin_index], address_pin_states[pin_index]);
			}
			// Set data in pins.
			data_converter.IntToStates(write_data[address], data_pin_states);
			for (int pin_index = 0; pin_index < data_bus_width; pin_index ++) {
				sim->ChildSet("test_ram", data_in_pin_names[pin_index], data_pin_states[pin_index]);
			}
			// Run the simulation for two ticks to get the F -> T clock transition to write the word.
			auto t1 = std::chrono::high_resolution_clock::now();
			if (address == 0) {
				sim->Run(2, true, false, false, true);
			} else {
				sim->Run(2, false, false, false, true);
			}
			auto t2 = std::chrono::high_resolution_clock::now();
			long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
			sub_total_1 += microseconds;
		}
		
		
		// Zero the data in pins.
		for (int pin_index = 0; pin_index < data_bus_width; pin_index ++) {
			sim->ChildSet("test_ram", data_in_pin_names[pin_index], false);
		}
		
		// Switch from write to read mode and read the data back out into a vector from the top.
		sim->ChildSet("test_ram", "write", false);
		sim->ChildSet("test_ram", "read", true);

		std::vector<bool> read_states = {};
		std::vector<int> read_data = {};
		for (int address = 0; address < address_max; address ++) {
			read_data.push_back(0);
		}
		
		// We need a pointer to the Device.
		Component* ram = sim->GetChildComponentPointer("test_ram");

		// Run through the addresses from the top.
		long long sub_total_2 = 0;
		for (int address = address_max - 1; address >= 0; address --) {
			// Set address pins.
			address_converter.IntToStates(address, address_pin_states);
			for (int pin_index = 0; pin_index < address_bus_width; pin_index ++) {
				sim->ChildSet("test_ram", address_pin_names[pin_index], address_pin_states[pin_index]);
			}
			// Run the simulation for two ticks to get the T -> F clock transition to read the word.
			auto t3 = std::chrono::high_resolution_clock::now();
			if (address == 0) {
				sim->Run(2, false, false, false, true);
			} else {
				sim->Run(2, false, false, false, true);
			}
			auto t4 = std::chrono::high_resolution_clock::now();
			long long microseconds = std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count();
			sub_total_2 += microseconds;
			// Capture the pin states.
			for (int pin_index = 0; pin_index < data_bus_width; pin_index ++) {
				read_states.emplace_back(ram->GetPinState(ram->GetPinPortIndex(data_out_pin_names[pin_index])));
			}
			// Convert to int and store.
			read_data[address] = data_converter.StatesToInt(read_states);
			read_states.clear();
			// If written and read values do not agree, break.
			if (read_data[address] != write_data[address]) {
				std::cout << "DATA MISMATCH." << std::endl;
				break;
			}
		}
		
		std::cout << "Run : " << run_index << std::endl;
		for (int i = 5; i >= 0; i --) {
			std::cout << "Address : " << i << "   Written : " << write_data[i] << "   Read : " << read_data[i] << std::endl;
		}
		long long total = sub_total_1 + sub_total_2;
		std::cout << "Total time (usec) : " << total << std::endl;
		std::cout << "--------------------------------------------------------------------------------"<< std::endl;
		run_times.push_back(total);
	}
	
	std::cout << std::endl << "Run times (usec)"<< std::endl;
	for (int i = 0; i < number_of_runs; i ++) {
		std::cout << i << ", " << run_times[i] << std::endl;
	}
	
	delete sim;
	
	return 0;
}

//~sim.ChildConnect("test_ram", {"d_out_7", "parent", "__ALL_STOP__"});
