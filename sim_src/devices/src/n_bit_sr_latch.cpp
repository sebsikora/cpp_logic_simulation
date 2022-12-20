// n_bit_sr_latch.cpp
#include <string>
#include <vector>

#include "c_structs.hpp"
#include "c_gate.hpp"
#include "c_device.hpp"

#include "sr_latch.h"            // Previously defined SR latch device
#include "n_bit_sr_latch.h"      // Our new device

N_Bit_SR_Latch::N_Bit_SR_Latch(Device* parent_device_pointer, std::string name, int latch_count, bool monitor_on, std::vector<StateDescriptor> in_pin_default_states) 
 : Device(parent_device_pointer, name, "n_bit_sr_latch", {"R_All"}, {}, monitor_on, in_pin_default_states) {
	if (latch_count > 0) {
		m_latch_count = latch_count;
	} else {
		m_latch_count = 1;
	}
	CreateBus(m_latch_count, "S_", Pin::Type::IN, in_pin_default_states);
	CreateBus(m_latch_count, "R_", Pin::Type::IN, in_pin_default_states);
	CreateBus(m_latch_count, "Out_", Pin::Type::OUT);
	Build();
	Stabilise();
}

void N_Bit_SR_Latch::Build() {
	for (int slice_index = 0; slice_index < m_latch_count; slice_index ++) {
		std::string latch_identifier = "sr_latch_" + std::to_string(slice_index);
		std::string or_identifier = "or_" + std::to_string(slice_index);
		std::string slice = std::to_string(slice_index);
		
		AddComponent(new SR_Latch(this, latch_identifier));
		AddGate(or_identifier, "or", {"input_0", "input_1"});
		
		Connect("S_" + slice, latch_identifier, "S");
		Connect("R_" + slice, or_identifier, "input_0");
		Connect("R_All", or_identifier, "input_1");
		ChildConnect(or_identifier, {latch_identifier, "R"});
		ChildConnect(latch_identifier, {"Out", "parent", "Out_" + slice});
	}
}