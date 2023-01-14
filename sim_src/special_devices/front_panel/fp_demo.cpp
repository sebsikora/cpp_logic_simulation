#include <FL/Fl.H>
#include <thread>
#include <atomic>
#include <functional>
#include <condition_variable>
#include <chrono>

#include "fp_components.h"

class GuiManager {
public:
	class DefaultGui : public Fl_Window
	{
	public:
		DefaultGui(int width, int height, std::string const& name) : Fl_Window(width, height)
		{
			copy_label(name.c_str());
			end();
		}
		~DefaultGui() {}
	};
	
	GuiManager()
	{
		m_guiRunning = 0;
		m_started = false;
	}
	~GuiManager() {}

	static void staticHideAllCallback(void* data)
	{
		while (Fl::first_window()) {	// Hide all shown windows.
			Fl::first_window()->hide();
		}
		//~((Fl_Window*)data)->hide();
	}

	void start();
	void stop();
	
private:
	void guiRuntime();

	std::mutex m_guiMutex;
	std::condition_variable m_guiCv;
	std::atomic_int m_guiRunning;
	Fl_Window* m_window;
	std::thread m_runtimeThread;
	std::atomic_bool m_started;
};

void GuiManager::start()
{
	if (m_started == false) {
		std::unique_lock<std::mutex> lock(m_guiMutex);
		
		m_runtimeThread = std::thread(&GuiManager::guiRuntime, this);

		while (m_guiRunning == 0) {
			m_guiCv.wait_for(lock, std::chrono::seconds(1));
		}

	}
}

void GuiManager::stop()
{
	if (m_guiRunning > 0) {		
		Fl::awake(GuiManager::staticHideAllCallback, (void*)m_window);
	}

	if (m_started == true) {
		m_runtimeThread.join();
		m_started = false;
		m_guiRunning = 0;
	}

	if (m_window != nullptr) {
		delete m_window;
	}
}

void GuiManager::guiRuntime()
{
	m_started = true;
	std::cout << "GUI thread running" << std::endl;
	
	Fl::lock();		/* "start" the FLTK lock mechanism */
	
	m_window = new DefaultGui(100, 100, "test");
	m_window->show();

	bool flagged = false;
	
	while (true) {
		m_guiRunning = Fl::wait();
		if (m_guiRunning <= 0) {
			break;
		} else {
			if (!flagged) {
				flagged = true;
				std::unique_lock<std::mutex> lock(m_guiMutex);
				m_guiCv.notify_all();
				std::cout << "Main thread notified" << std::endl;
			}
		}
	}
}

struct PanelConfig {
	std::string name;
	int width;
	int height;
	std::vector<std::string> params;
	bool echo;
};

class PanelManager {
public:
	PanelManager(std::string const& name, int width, int height, std::vector<std::string> const& params, bool echo) :
		m_panelConfig{name, width, height, params, echo},
		m_panel(nullptr)
	{}
	~PanelManager() {
		if (m_panel != nullptr) {
			delete m_panel;
		}
	}

	static void staticCreatePanelCallback(void* data) { ((PanelManager*)data)->createPanelCallback(); }

	void initialise();
	void open();
	void close();
	
private:
	void createPanelCallback();
	
	PanelConfig m_panelConfig;

	std::atomic<bool> m_guiBusy{true};
	std::mutex m_guiMutex;
	std::condition_variable m_guiCv;

	Panel* m_panel;
};

void PanelManager::createPanelCallback()
{	
	if (m_panel == nullptr) {
		std::cout << "About to instantiate panel" << std::endl;
		m_panel = new Panel(m_panelConfig.name,
							m_panelConfig.width,
							m_panelConfig.height,
							m_panelConfig.params,
							m_panelConfig.echo);

		std::cout << "Panel instantiated" << std::endl;
	}

	std::unique_lock<std::mutex> lock(m_guiMutex);
	m_guiBusy = false;
	m_guiCv.notify_all();
}

void PanelManager::initialise()
{
	m_guiBusy = true;

	std::unique_lock<std::mutex> lock(m_guiMutex);

	std::cout << Fl::awake(PanelManager::staticCreatePanelCallback, (void*)(this)) << std::endl;

	while (m_guiBusy == true) {
		m_guiCv.wait(lock);
	}
	
	if (m_panel != nullptr) {
		m_panel->setIndicatorsValue(0, 4);
	}
}

void PanelManager::open()
{
	if (m_panel != nullptr) {
		m_panel->open();
	}
}

void PanelManager::close()
{
	if (m_panel != nullptr) {
		m_panel->close();
	}
}

int main(int argc, char **argv)
{
	
	GuiManager gm;		// Member of Simulation, constructor called in Simulation constructor.
	
	PanelManager dd("test",	// Member of SpecialDevice class
				   300,
				   180,
				   {"toggle, toggle, 6, 0, 0, 70, 30, 30",
					"momentary, momentary, 6, 0, 40, 70, 30, 30",
					"radio, radio, 6, A. B. C. D. E. F, 0, 80, 70, 30, 30",
					"indicator, lamps, 6, 0, 120, 70, 30, 30"
				   },
				   true);

	gm.start();			// Called during setup phase at start of Simulation::run() call.
	
	std::this_thread::sleep_for(std::chrono::milliseconds(2000));

	dd.initialise();	// Called during setup phase at start of Simulation::run() call.
	dd.open();

	std::this_thread::sleep_for(std::chrono::milliseconds(6000));

	dd.close();			// Called at end of Simulation::run()
	
	gm.stop();			// Called at end of Simulation::run()
	
	return 0;
}
