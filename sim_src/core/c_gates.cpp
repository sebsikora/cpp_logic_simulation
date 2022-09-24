#include <vector>
#include <string>

#include "c_gates.hpp"

class Device;

AndGate::AndGate(Device* parent_device_pointer, std::string const& gate_name, std::vector<std::string> in_pin_names, bool monitor_on) {
	Configure(parent_device_pointer, gate_name, "and", in_pin_names, monitor_on);
}

inline bool AndGate::Operate(void) {
	bool output = true;
	for (int i = 0; i < m_in_pin_count; ++i) {
		output &= m_pins[i].state;
	}
	return output;
}

NandGate::NandGate(Device* parent_device_pointer, std::string const& gate_name, std::vector<std::string> in_pin_names, bool monitor_on) {
	Configure(parent_device_pointer, gate_name, "nand", in_pin_names, monitor_on);
}

inline bool NandGate::Operate(void) {
	bool output = true;
	for (int i = 0; i < m_in_pin_count; ++i) {
		output &= m_pins[i].state;
	}
	return !output;
}

OrGate::OrGate(Device* parent_device_pointer, std::string const& gate_name, std::vector<std::string> in_pin_names, bool monitor_on) {
	Configure(parent_device_pointer, gate_name, "or", in_pin_names, monitor_on);
}

inline bool OrGate::Operate(void) {
	bool output = false;
	for (int i = 0; i < m_in_pin_count; ++i) {
		output |= m_pins[i].state;
	}
	return output;
}

NorGate::NorGate(Device* parent_device_pointer, std::string const& gate_name, std::vector<std::string> in_pin_names, bool monitor_on) {
	Configure(parent_device_pointer, gate_name, "nor", in_pin_names, monitor_on);
}

inline bool NorGate::Operate(void) {
	bool output = false;
	for (int i = 0; i < m_in_pin_count; ++i) {
		output |= m_pins[i].state;
	}
	return !output;
}

Inverter::Inverter(Device* parent_device_pointer, std::string const& gate_name, bool monitor_on) {
	Configure(parent_device_pointer, gate_name, "not", {"input"}, monitor_on);
}

inline bool Inverter::Operate(void) {
	return !m_pins[0].state;
}
