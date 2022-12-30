#include <FL/Fl.H>
#include <thread>
#include <atomic>

#include "fp_components.h"

class GuiManager {
public:
	GuiManager() { m_windowOpen = false; }
	~GuiManager() {}

	static void staticHideAllCallback(void* data)
	{
		while (Fl::first_window()) {
			Fl::first_window()->hide();
		}
		//~*((std::atomic_bool*)data) = false;
	}

	void start();
	void stop();
	
private:
	void guiRuntime();

	std::atomic_bool m_windowOpen;
	Fl_Window* m_window;
	std::thread m_runtimeThread;
};

void GuiManager::start()
{
	m_runtimeThread = std::thread(&GuiManager::guiRuntime, this);
	while (m_windowOpen == false) {}
}

void GuiManager::stop()
{
	//~Fl::awake(GuiManager::staticHideAllCallback, (void*)(&m_windowOpen));
	Fl::awake(GuiManager::staticHideAllCallback, 0);
	m_runtimeThread.join();
}

void GuiManager::guiRuntime()
{
	std::cout << "GUI thread running" << std::endl;
	
	Fl::lock();		/* "start" the FLTK lock mechanism */
	
	m_window = new Fl_Window(100, 100, "test");
	m_window->end();
	m_window->show();

	m_windowOpen = true;
	std::cout << "m_windowOpen = true" << std::endl;
	
	while (Fl::wait() > 0) {}
}

PanelManager* pm;

struct PanelConfig {
	std::string name;
	int width;
	int height;
	std::vector<std::string> params;
	bool echo;
	std::atomic_bool busy;
};

void openPanelCallback(void* data)
{	
	PanelConfig* pc = (PanelConfig*)data;

	std::cout << "About to instantiate panel" << std::endl;
	
	pm = new PanelManager(pc->name,
						  pc->width,
						  pc->height,
						  pc->params,
						  pc->echo);

	std::cout << "Panel instantiated" << std::endl;
		
	pc->busy = false;
}

int main(int argc, char **argv)
{
	
	GuiManager gm;
	gm.start();
	
	PanelConfig pc {"test",
				   300,
				   180,
				   {"toggle, toggle, 6, 0, 0, 70, 30, 30",
					"momentary, momentary, 6, 0, 40, 70, 30, 30",
					"radio, radio, 6, A. B. C. D. E. F, 0, 80, 70, 30, 30",
					"indicator, lamps, 6, 0, 120, 70, 30, 30"
				   },
				   true};

	pc.busy = true;

	std::cout << Fl::awake(openPanelCallback, (void*)(&pc)) << std::endl;

	while (pc.busy == true) { }
	
	pm->setIndicatorsValue(0, 4);

	Fl::awake(PanelManager::staticRequestShowCallback, (void*)(pm));

	std::this_thread::sleep_for(std::chrono::milliseconds(2000));

	Fl::awake(PanelManager::staticRequestHideCallback, (void*)(pm));

	std::this_thread::sleep_for(std::chrono::milliseconds(2000));
	
	gm.stop();
	
	return 0;
}


//~pm = new PanelManager("test",
							//~300,
							//~180,
							//~{"toggle, toggle, 6, 0, 0, 70, 30, 30",
							 //~"momentary, momentary, 6, 0, 40, 70, 30, 30",
							 //~"radio, radio, 6, A. B. C. D. E. F, 0, 80, 70, 30, 30",
							 //~"indicator, lamps, 6, 0, 120, 70, 30, 30"
							//~},
							//~true);

//~void guiRuntime()
//~{
	//~Fl::lock();

	//~std::cout << "GUI thread running" << std::endl;

	//~Fl_Window* w = new Fl_Window(100, 100, "test");
	//~w->end();
	//~w->show();

	//~g_ready = true;
	
	//~while (Fl::wait() > 0) {}
//~}
