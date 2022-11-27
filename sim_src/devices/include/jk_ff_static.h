#include "c_structs.hpp"
#include "c_gates.hpp"

class JK_FF_static : public Device {
	public:
		JK_FF_static(Device* parent_device_pointer, std::string name, bool monitor_on = false, std::vector<StateDescriptor> input_default_states = {});
		virtual ~JK_FF_static() { }
		void Build(void) override;
	private:
		NandGate m_nand1;
		NandGate m_nand2;
		NandGate m_nand3;
		NandGate m_nand4;
		NandGate m_nand5;
		NandGate m_nand6;
		NandGate m_nand7;
		NandGate m_nand8;
		Inverter m_not1;
};

class Four_Bit_Counter_static : public Device {
	public:
		Four_Bit_Counter_static(Device* parent_device_pointer, std::string name, bool monitor_on = false, std::vector<StateDescriptor> input_default_states = {});
		virtual ~Four_Bit_Counter_static() { }
		void Build(void) override;
	private:
		JK_FF_static m_jkff0;
		JK_FF_static m_jkff1;
		JK_FF_static m_jkff2;
		JK_FF_static m_jkff3;
		AndGate m_and0;
		AndGate m_and1;
		AndGate m_and2;
};
