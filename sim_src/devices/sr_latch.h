// sr_latch.h

#include "c_core.h"			// Core simulator functionality

class SR_Latch : public Device {
	public:
		SR_Latch(Device* parent_device_pointer, std::string name, bool monitor_on = false,
		         std::vector<state_descriptor> input_default_states = {});
		void Build(void);
};

