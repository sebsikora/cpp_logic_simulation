#ifndef LSIM_NBITSR_H
#define LSIM_NBITSR_H

#include "c_structs.hpp"
#include "c_device.hpp"

class N_Bit_SR_Latch : public Device {
	public:
		N_Bit_SR_Latch(Device* parent_device_pointer, std::string name, int latch_count, bool monitor_on = false,
		              std::vector<StateDescriptor> input_default_states = {});
		void Build(void);
		int m_latch_count;
};

#endif	// LSIM_NBITSR_H
