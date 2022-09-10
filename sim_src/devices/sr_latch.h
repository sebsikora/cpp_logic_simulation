#ifndef LSIM_SRLATCH_H
#define LSIM_SRLATCH_H

#include "c_structs.hpp"
#include "c_device.hpp"

class SR_Latch : public Device {
	public:
		SR_Latch(Device* parent_device_pointer, std::string name, bool monitor_on = false,
		         std::vector<state_descriptor> input_default_states = {});
		void Build(void);
};

#endif	// LSIM_SRLATCH_H
