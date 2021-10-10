#include "c_core.h"

#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <chrono>

int main() {
	using std::chrono::high_resolution_clock;

	unsigned long master_accumulator = 0;
	
	// Test config.
	std::vector<int> number_of_pins = {1, 2, 4, 8, 16, 32, 64, 128};
	int number_of_runs = 300;
	int number_of_random_lookups = 100000;
	
	// Create pin names.
	std::vector<std::string> pin_names;
	std::vector<size_t> pin_hashes;
	for (int i = 0; i < number_of_pins.back(); i ++) {
		std::string pin_name = "pin_number_" + std::to_string(i);
		pin_names.push_back(pin_name);
		pin_hashes.push_back(std::hash<std::string>{}(pin_name));
	}
	
	std::vector<double> run_map_times;
	std::vector<double> run_table_times;
	for (const auto& pin_count: number_of_pins) {
		std::vector<size_t> pin_hash_table;
		std::vector<pin> pin_table;
		pin_table.reserve(pin_count);
		
		std::unordered_map<std::size_t, pin> pin_map;
		
		for (int i = 0; i < pin_count; i ++) {
			std::string pin_name = pin_names[i];
			std::size_t pin_name_hash = pin_hashes[i];
			int random_int = rand() % 10;
			pin new_pin = {pin_name, pin_name_hash, random_int, false, false};
			
			// In-order insertion into pin_hash_table.
			std::vector<size_t>::iterator this_entry = std::upper_bound(pin_hash_table.begin(), pin_hash_table.end(), pin_name_hash);
			if (this_entry != pin_hash_table.end()) {
				// If not first entry, get insertion index.
				int index = std::distance(pin_hash_table.begin(), this_entry);
				pin_hash_table.insert(pin_hash_table.begin() + index, pin_name_hash);
				pin_table.insert(pin_table.begin() + index, new_pin);
			} else {
				// First entry.
				pin_table.push_back(new_pin);
				pin_hash_table.push_back(pin_name_hash);
			}

			// Key-based insertion into pin_map.
			pin_map[pin_name_hash] = new_pin;
		}
		
		//~std::cout << std::endl << "Hashes in hash table" << std::endl;
		//~for (const auto& hash_table_entry: pin_hash_table) {
			//~std::cout << hash_table_entry << std::endl;
		//~}
		//~std::cout << std::endl << "Pins in pin table (by name hash)" << std::endl;
		//~for (const auto& pin_table_entry: pin_table) {
			//~std::cout << std::hash<std::string>{}(pin_table_entry.name) << std::endl;
		//~}
		//~std::cout << std::endl;

		std::vector<double> this_run_map_time;
		std::vector<double> this_run_table_time;
		std::cout << std::to_string(pin_count) << " Pins, " << std::to_string(number_of_runs) << " runs of " << std::to_string(number_of_random_lookups) << " random pin lookups" << std::endl;
		for (int run_index = 0; run_index < number_of_runs; run_index ++) {
			//~std::cout << "Starting run : " << std::to_string(run_index) << std::endl;
			std::vector<std::string> map_lookup_names;
			std::vector<std::string> hash_table_lookup_names;
			std::vector<int> lookups;
			for (int l = 0; l < number_of_random_lookups; l ++) {
				lookups.push_back(rand() % pin_count);		// Random int in range pin_count.
			}
			auto t1 = high_resolution_clock::now();						// Start timer.
			for (int lookup_index = 0; lookup_index < number_of_random_lookups; lookup_index ++) {
				size_t lookup_pin_name_hash = pin_hashes[lookups[lookup_index]];
				// Lookup pin in map.
				pin* map_lookup_pin = &pin_map[lookup_pin_name_hash];
				master_accumulator += (unsigned long)map_lookup_pin->direction;
				map_lookup_pin->state = map_lookup_pin->state;
			}
			auto t2 = high_resolution_clock::now();						// Stop timer.
			std::chrono::duration<double, std::milli> ms_int_map = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
			this_run_map_time.push_back(ms_int_map.count());
			
			auto t3 = high_resolution_clock::now();						// Start timer.
			for (int lookup_index = 0; lookup_index < number_of_random_lookups; lookup_index ++) {
				size_t lookup_pin_name_hash = pin_hashes[lookups[lookup_index]];
				// Lookup pin via hash table.
				std::vector<size_t>::iterator this_entry = std::find(pin_hash_table.begin(), pin_hash_table.end(), lookup_pin_name_hash);
				if (this_entry != pin_hash_table.end()) {
					// If present...
					int hash_table_index = std::distance(pin_hash_table.begin(), this_entry);
					pin* hash_table_lookup_pin = &pin_table[hash_table_index];
					master_accumulator += (unsigned long)hash_table_lookup_pin->direction;
					hash_table_lookup_pin->state = !hash_table_lookup_pin->state;
				} else {
					master_accumulator += 0;
				}
			}
			auto t4 = high_resolution_clock::now();						// Stop timer.
			std::chrono::duration<double, std::milli> ms_int_vec = std::chrono::duration_cast<std::chrono::milliseconds>(t4 - t3);
			this_run_table_time.push_back(ms_int_vec.count());
		}
		// Calculate average of times
		double accumulator = 0.0;
		for (const auto& value: this_run_map_time) {
			accumulator += value;
		}
		run_map_times.push_back(accumulator/(double)number_of_runs);

		accumulator = 0.0;
		for (const auto& value: this_run_table_time) {
			accumulator += value;
		}
		run_table_times.push_back(accumulator/(double)number_of_runs);
	}
	for (int k = 0; k < number_of_pins.size(); k ++) {
		std::cout << std::to_string(number_of_pins[k]) << " pins : " << std::to_string(run_map_times[k]) << "            " << std::to_string(run_table_times[k]) << std::endl;
	}
}
