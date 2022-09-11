#ifndef LSIM_CORE_GATES_HPP
#define LSIM_CORE_GATES_HPP

#include "c_gate.hpp"

class Device;

class AndGate : public Gate {
	public:
		AndGate(Device* parent_device_pointer, std::string const& gate_name, std::vector<std::string> in_pin_names, bool monitor_on = false);
		virtual ~AndGate() { }

	protected:
		virtual bool Operate(void) final override;
};

class NandGate : public Gate {
	public:
		NandGate(Device* parent_device_pointer, std::string const& gate_name, std::vector<std::string> in_pin_names, bool monitor_on = false);
		virtual ~NandGate() { }

	protected:
		virtual bool Operate(void) final override;
};

class OrGate : public Gate {
	public:
		OrGate(Device* parent_device_pointer, std::string const& gate_name, std::vector<std::string> in_pin_names, bool monitor_on = false);
		virtual ~OrGate() { }

	protected:
		virtual bool Operate(void) final override;
};

class NorGate : public Gate {
	public:
		NorGate(Device* parent_device_pointer, std::string const& gate_name, std::vector<std::string> in_pin_names, bool monitor_on = false);
		virtual ~NorGate() { }

	protected:
		virtual bool Operate(void) final override;
};

class Inverter : public Gate {
	public:
		Inverter(Device* parent_device_pointer, std::string const& gate_name, bool monitor_on = false);
		virtual ~Inverter() { }

	protected:
		virtual bool Operate(void) final override;
};

#endif	// LSIM_CORE_GATES_HPP
