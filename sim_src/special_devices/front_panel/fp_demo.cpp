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
		~DefaultGui() { }
	};
	
	GuiManager() { m_guiRunning = 0; }
	~GuiManager() {}

	static void staticHideAllCallback(void* data)
	{
		//~while (Fl::first_window()) {
			//~Fl::first_window()->hide();
		//~}
		//~*((std::atomic_bool*)data) = false;
		((Fl_Window*)data)->hide();
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
};

void GuiManager::start()
{
	std::unique_lock<std::mutex> lock(m_guiMutex);
	
	m_runtimeThread = std::thread(&GuiManager::guiRuntime, this);

	while (m_guiRunning == 0) {
		m_guiCv.wait_for(lock, std::chrono::seconds(1));
	}
}

void GuiManager::stop()
{
	//~Fl::awake(GuiManager::staticHideAllCallback, (void*)(&m_windowOpen));
	Fl::awake(GuiManager::staticHideAllCallback, (void*)m_window);
	m_runtimeThread.join();
}

void GuiManager::guiRuntime()
{
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

PanelManager* pm;

//~struct PanelConfig {
	//~std::string name;
	//~int width;
	//~int height;
	//~std::vector<std::string> params;
	//~bool echo;
	//~std::atomic<bool> busy{true};
	//~std::mutex mtx;
	//~std::condition_variable cv;
//~};

//~void openPanelCallback(void* data)
//~{	
	//~PanelConfig* pc = (PanelConfig*)data;

	//~std::cout << "About to instantiate panel" << std::endl;
	
	//~pm = new PanelManager(pc->name,
						  //~pc->width,
						  //~pc->height,
						  //~pc->params,
						  //~pc->echo);

	//~std::cout << "Panel instantiated" << std::endl;

	//~std::unique_lock<std::mutex> lock(pc->mtx);
	//~pc->busy = false;
	//~pc->cv.notify_all();
//~}

class DummyDevice {
public:
	DummyDevice(std::string const& name, int width, int height, std::vector<std::string> const& params, bool echo) :
		m_panelConfig{name, width, height, params, echo},
		m_panelManager{nullptr}
	{
		m_panelConfig.busy = true;
		std::unique_lock<std::mutex> lock(m_panelConfig.mtx);
		
		std::pair<std::unique_ptr<PanelManager>*, PanelConfig*> p{&m_panelManager, &m_panelConfig};
		Fl::awake(PanelManager::staticCreatePanelCallback, (void*)(&p));

		while (m_panelConfig.busy == true) {
			m_panelConfig.cv.wait(lock);
		}

		if (m_panelManager != nullptr) {
			m_panelManager->setIndicatorsValue(0, 4);
		}
	}
	~DummyDevice() { }

	void show();
	void hide();
	
private:
	void openPanelCallback();
	
	PanelConfig m_panelConfig;

	std::atomic<bool> m_guiBusy{true};
	std::mutex m_guiMutex;
	std::condition_variable m_guiCv;

	std::unique_ptr<PanelManager> m_panelManager;
};

//~void DummyDevice::openPanelCallback()
//~{	
	//~std::cout << "About to instantiate panel" << std::endl;
	
	//~m_panelManager = new PanelManager(m_panelConfig.name,
						  //~m_panelConfig.width,
						  //~m_panelConfig.height,
						  //~m_panelConfig.params,
						  //~m_panelConfig.echo);

	//~std::cout << "Panel instantiated" << std::endl;

	//~std::unique_lock<std::mutex> lock(m_guiMutex);
	//~m_guiBusy = false;
	//~m_guiCv.notify_all();
//~}

void DummyDevice::show()
{
	//~std::cout << Fl::awake(PanelManager::staticRequestShowCallback, (void*)(m_panelManager)) << std::endl;
	m_panelManager->open();
}

void DummyDevice::hide()
{
	//~std::cout << Fl::awake(PanelManager::staticRequestHideCallback, (void*)(m_panelManager)) << std::endl;
	m_panelManager->close();
}

int main(int argc, char **argv)
{
	
	GuiManager gm;
	gm.start();

	//~PanelConfig pc {"test",
				   //~300,
				   //~180,
				   //~{"toggle, toggle, 6, 0, 0, 70, 30, 30",
					//~"momentary, momentary, 6, 0, 40, 70, 30, 30",
					//~"radio, radio, 6, A. B. C. D. E. F, 0, 80, 70, 30, 30",
					//~"indicator, lamps, 6, 0, 120, 70, 30, 30"
				   //~},
				   //~true};
	DummyDevice dd("test",
				   300,
				   180,
				   {"toggle, toggle, 6, 0, 0, 70, 30, 30",
					"momentary, momentary, 6, 0, 40, 70, 30, 30",
					"radio, radio, 6, A. B. C. D. E. F, 0, 80, 70, 30, 30",
					"indicator, lamps, 6, 0, 120, 70, 30, 30"
				   },
				   true);

	std::this_thread::sleep_for(std::chrono::milliseconds(2000));

	dd.show();

	std::this_thread::sleep_for(std::chrono::milliseconds(5000));

	dd.hide();
	
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
