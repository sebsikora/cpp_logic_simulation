#ifndef TOGGLE_SWITCHES_HPP
#define TOGGLE_SWITCHES_HPP

#include <string>
#include <vector>
#include <algorithm>
#include <array>
#include <iostream>

#include <FL/Fl.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>

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

protected:
	virtual void switchChangeCallback(Fl_Widget* w);
	
	int m_index = 0;
	std::vector<Fl_Button*> m_buttons;
	unsigned long m_value = 0ul;
	Fl_Callback* m_valueChangeCallback = 0;
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
	RadioSwitches(std::string const& name, int count, int x, int y, int labelWidth, int buttonWidth, int height);
	virtual ~RadioSwitches() {}

protected:
	void switchChangeCallback(Fl_Widget* w) override;

private:
	Fl_Box* m_label;
};

class PanelManager : public Fl_Group 
{
public:
	PanelManager(std::string const& name, int x, int y, int width, int height);
	virtual ~PanelManager() {}
	
	static void staticSwitchChangeCallback(Fl_Widget* w, void* f) {((PanelManager*)f)->switchChangeCallback(w); }

	void addPanelWidget(std::string const& widgetParams);

private:
	void switchChangeCallback(Fl_Widget* w);

	std::vector<Fl_Widget*> m_widgets;
};

#endif // TOGGLE_SWITCHES_HPP
