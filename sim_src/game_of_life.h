#ifndef LSIM_GAME_OF_LIFE_H
#define LSIM_GAME_OF_LIFE_H

#include <string>					// std::string.
#include <vector>					// std::vector
#include <unordered_map>			// std::unordered_map

#include "c_core.h"					// Core simulator functionality

class GameOfLife_Cell_Decider : public Device {
	public:
		GameOfLife_Cell_Decider(Device* parent_device_pointer, std::string name, bool monitor_on = false, std::unordered_map<std::string, bool> in_pin_default_states = {});
		void Build(void);
};

class GameOfLife_Cell_SiblingSelector : public Device {
	public:
		GameOfLife_Cell_SiblingSelector(Device* parent_device_pointer, std::string name, bool monitor_on = false, std::unordered_map<std::string, bool> in_pin_default_states = {});
		void Build(void);
};

class GameOfLife_Cell : public Device {
	public:
		GameOfLife_Cell(Device* parent_device_pointer, std::string name, bool monitor_on = false, std::unordered_map<std::string, bool> in_pin_default_states = {});
		void Build(void);
};

#endif
