#include <string>					// std::string.
#include <iostream>					// std::cout, std::endl.
#include <vector>					// std::vector
#include <algorithm>				// std::sort
#include <cstdlib>					// rand()

#include "c_monitor.hpp"
#include "c_structs.hpp"
#include "c_device.hpp"
#include "c_sim.hpp"

#include "utils.h"
#include "strnatcmp.h"
#include "colors.h"

Monitor::~Monitor() {
#ifdef VERBOSE_DTORS
	std::cout << "Monitor dtor for " << GetFullName() << " @ " << this << std::endl;
#endif
	PurgeComponent();
}

Monitor::Monitor(Device* parent_device_pointer, std::string const& name, std::vector<std::string> in_pin_names, bool monitor_on) {
	m_device_flag = false;
	m_name = name;
	m_parent_device_pointer = parent_device_pointer;
	m_top_level_sim_pointer = m_parent_device_pointer->GetTopLevelSimPointer();
	m_CUID = m_top_level_sim_pointer->GetNewCUID();
	m_nesting_level = m_parent_device_pointer->GetNestingLevel() + 1;
	m_component_type = "monitor";

	m_in_pin_count = in_pin_names.size();

	std::sort(in_pin_names.begin(), in_pin_names.end(), compareNat);
	int new_pin_port_index = 0;
	for (const auto& pin_name : in_pin_names) {
		// Assign random states to Gate inputs.
		bool temp_bool = rand() > (RAND_MAX / 2);
		Pin new_in_pin = {pin_name, Pin::Type::IN, temp_bool, false, new_pin_port_index, {false, false}};
		m_pins.push_back(new_in_pin);
		new_pin_port_index ++;
	}
}

void Monitor::Reset() {
	/* Empty for Monitor */
}

void Monitor::Initialise() {
	/* Empty for Monitor */
}

void Monitor::Connect(std::vector<std::string> connection_parameters) {
	/* Empty for Monitor */
}

void Monitor::Propagate() {
	/* Empty for Monitor */
}

void Monitor::PurgeInboundConnections(Component* target_component_pointer) {
	/* Empty for Monitor */
}

void Monitor::PurgeOutboundConnections() {
	/* Empty for Monitor */
}

void Monitor::Set(const int pin_port_index, const bool state_to_set) {
	Pin* this_pin = &m_pins[pin_port_index];
	if (this_pin->state != state_to_set) {
		std::string message = std::string(KBLD) + KGRN + std::to_string(m_top_level_sim_pointer->GetGlobalTickIndex()) + RST + ": " + KBLD + KYEL + "MONITOR" + RST + ": " + KBLD + GetFullName() + RST + " terminal " + KBLD + this_pin->name + RST + " set from " + BoolToChar(this_pin->state) + " to " + BoolToChar(state_to_set);
		m_top_level_sim_pointer->LogMessage(message);
		this_pin->state = state_to_set;
	}
}

void Monitor::PrintPinStates(int max_levels) {
	std::cout << GetFullName() << ": [";
	for (const auto& in_pin_name: GetSortedInPinNames()) {
		int in_pin_port_index = GetPinPortIndex(in_pin_name);
		std::cout << " " << BoolToChar(m_pins[in_pin_port_index].state) << " ";
	}
}

void Monitor::ReportUnConnectedPins() {
#ifdef VERBOSE_SOLVE
	std::string message = "Checking pins for " + GetFullName();
	m_top_level_sim_pointer->LogMessage(message);
#endif
	for (const auto& this_pin : m_pins) {
		if (!this_pin.driven.in) {
			// Log undriven Monitor in pin.
			std::string build_error = "Monitor " + GetFullName() + " in pin " + this_pin.name + " is not driven by any Component.";
			m_top_level_sim_pointer->LogError(build_error);
		}
	}
}

void Monitor::PurgeComponent() {
#ifdef VERBOSE_DTORS
	std::string header;
	header =  "Purging -> MONITOR : " + GetFullName() + " @ " + PointerToString(static_cast<void*>(this));
	std::cout << GenerateHeader(header) << std::endl;
#endif
	if (!(m_parent_device_pointer->GetDeletionFlag())) {
		// First  - Ask parent device to purge all local references to this Gate...
		//			If we are deleting this Component because we are in the process of deleting
		//			it's parent, we do not need to do this.
		m_parent_device_pointer->PurgeChildConnections(this);
	}
	if (!(m_top_level_sim_pointer->GetDeletionFlag())) {
		// Second - Clear component entry from parent device's m_components (in case this monitor has been added manually...)
		m_parent_device_pointer->PurgeChildComponentIdentifiers(this);
	}
#ifdef VERBOSE_DTORS
	header =  "MONITOR : " + GetFullName() + " @ " + PointerToString(static_cast<void*>(this)) + " -> Purged.";
	std::cout << GenerateHeader(header) << std::endl;
#endif
	// - It should now be safe to delete this object -
}



