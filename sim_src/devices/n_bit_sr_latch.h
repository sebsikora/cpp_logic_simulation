// n_bit_sr_latch.h

#include "c_core.h"			// Core simulator functionality

class N_Bit_SR_Latch : public Device {
	public:
		N_Bit_SR_Latch(Device* parent_device_pointer, std::string name, int latch_count, bool monitor_on = false,
		              std::vector<state_descriptor> input_default_states = {});
		void Build(void);
		int m_latch_count;
};
