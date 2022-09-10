#ifndef LSIM_QSRLATCH_H
#define LSIM_QSRLATCH_H

#include "c_structs.hpp"
#include "c_device.hpp"

class Quad_SR_Latch : public Device {
	public:
		Quad_SR_Latch(Device* parent_device_pointer, std::string name, bool monitor_on = false,
		              std::vector<state_descriptor> input_default_states = {});
		void Build(void);
};

#endif	// LSIM_QSRLATCH_H
