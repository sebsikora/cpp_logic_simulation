#ifndef TOGGLE_SWITCHES_HPP
#define TOGGLE_SWITCHES_HPP

#include <string>
#include <vector>
#include <algorithm>
#include <array>
#include <iostream>
#include <mutex>
#include <atomic>
#include <condition_variable>

#include <FL/Fl.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Box.H>

class Indicators : public Fl_Group
{
public:
	Indicators(int x, int y, int width, int height) : Fl_Group(x, y, width, height) {}
	virtual ~Indicators() {}

	virtual void setValue(unsigned long value) {}

protected:
	std::vector<Fl_Widget*> m_indicators;
	unsigned long m_value = 0ul;

};

class Switches : public Fl_Group
{
public:
	Switches(int x, int y, int width, int height) : Fl_Group(x, y, width, height) {}
	virtual ~Switches() {}
	
	void setValueChangeCallback(Fl_Callback* valueChangeCallback)
	{
		m_valueChangeCallback = valueChangeCallback;
	}
	
	virtual unsigned long getValue()
	{
		return m_value;
	}

	static void staticSwitchChangeCallback(Fl_Widget* w, void* f) {((Switches*)f)->switchChangeCallback(w); }
	virtual void switchChangeCallback(Fl_Widget* w);
	
protected:
	std::vector<Fl_Widget*> m_buttons;
	unsigned long m_value = 0ul;
	Fl_Callback* m_valueChangeCallback = 0;
};

class BasicIndicators : public Indicators
{
public:
	BasicIndicators(std::string const& name, int count, int x, int y, int labelWidth, int indicatorWidth, int height);
	virtual ~BasicIndicators() {}
	
	void setValue(unsigned long value) override;
	
private:
	Fl_Box* m_label;
};

class ToggleSwitches : public Switches
{	
public:
	ToggleSwitches(std::string const& name, int count, int x, int y, int labelWidth, int buttonWidth, int height);
	virtual ~ToggleSwitches() {}
	
private:	
	Fl_Box* m_label;
};

class MomentarySwitches : public Switches
{	
public:
	MomentarySwitches(std::string const& name, int count, int x, int y, int labelWidth, int buttonWidth, int height);
	virtual ~MomentarySwitches() {}
	
private:
	Fl_Box* m_label;
};

class RadioSwitches : public Switches
{	
public:
	RadioSwitches(std::string const& name, int count, std::vector<std::string> const& names, int x, int y, int labelWidth, int buttonWidth, int height);
	virtual ~RadioSwitches() {}

protected:
	void switchChangeCallback(Fl_Widget* w) override;
private:
	Fl_Box* m_label;
};

class Panel : public Fl_Window
{
public:
	Panel(std::string const& name, int width, int height, std::vector<std::string> const& panelConfig, bool echo = false);
	virtual ~Panel() { close(); }
	
	static void staticSwitchChangeCallback(Fl_Widget* w, void* f) {((Panel*)f)->switchChangeCallback(w); }
	static void staticPromptedHideCallback(Fl_Widget* w, void* f) {((Panel*)f)->promptedHideCallback(); }
	
	static void staticRequestShowCallback(void* data) {((Panel*)data)->requestShowCallback(); }
	static void staticRequestHideCallback(void* data) {((Panel*)data)->requestHideCallback(); }

	void addPanelWidget(std::string const& widgetParams);

	void open();
	std::atomic_bool const& isOpen() const;
	void close();

	bool haveSwitchesChanged() const;
	std::vector<int> getChangedSwitchesIndices();
	unsigned long getChangedSwitchesValue(int widgetIndex) const;

	void setIndicatorsValue(int indicatorIndex, unsigned long value);

	std::mutex m_valueChangeMutex;

private:
	void switchChangeCallback(Fl_Widget* w);
	void switchesChanged(int widgetIndex, unsigned long value);

	void promptedHideCallback();
	void requestShowCallback();
	void requestHideCallback();

	bool m_echo;

	std::atomic_bool m_panelOpen;
	std::mutex m_panelMutex;
	std::condition_variable m_panelCv;
	
	std::vector<Fl_Widget*> m_switchesWidgets;
	bool m_switchesHaveChanged = false;
	std::vector<int> m_changedSwitches;
	std::vector<unsigned long> m_switchesWidgetValues;
	std::vector<Fl_Widget*> m_indicatorsWidgets;
	std::atomic_bool m_busy;

};

#endif // TOGGLE_SWITCHES_HPP
