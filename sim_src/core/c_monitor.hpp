
#define LSIM_CORE_MONITOR_HPP

#include <string>
#include <vector>

#include "c_structs.hpp"
#include "c_comp.hpp"

class Device;

// Pin Monitor Component sub-class.
class Monitor : public Component {
	public:
		Monitor(Device* parent_device_pointer, std::string const& name, std::vector<std::string> in_pin_names, bool monitor_on);
		virtual ~Monitor();

		// Override Component virtual methods.
		void Initialise(void) override;
		void Connect(std::vector<std::string> connection_parameters) override;
		void Set(const int pin_port_index, const bool state_to_set) override;
		void Propagate(void) override;
		void PrintPinStates(int max_levels) override;
		void ReportUnConnectedPins(void) override;
		void Reset(void) override;
		void PurgeComponent(void) override;
		void PurgeInboundConnections(Component* target_component_pointer) override;
		void PurgeOutboundConnections(void) override;

	private:
		void PrintPinChange(void);

		int m_in_pin_count;
};
		
