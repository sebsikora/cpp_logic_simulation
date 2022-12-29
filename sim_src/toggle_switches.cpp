#include <iostream>
#include <sstream>
#include <algorithm>
#include <string>

#include "toggle_switches.h"

#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>

void Switches::switchChangeCallback(Fl_Widget* w)
{
	Fl_Button* button = (Fl_Button*)w;
	auto it = std::find(m_buttons.begin(), m_buttons.end(), button);

	if (it != m_buttons.end()) {
		int index = it - m_buttons.begin();
		unsigned long bit = button->value();
		
		if (bit) {
			m_value |= (1ul << index);
		} else {
			m_value &= ~(1ul << index);
		}
		
		std::cout << "ToggleSwitches value is " << std::to_string(m_value) << std::endl;

		if (m_valueChangeCallback != 0) {
			m_valueChangeCallback(this, (void*)parent());
		}
	}
}

PanelManager::PanelManager(std::string const& name, int x, int y, int width, int height) :
	Fl_Group(x, y, width, height)
{
	addPanelWidget("toggle, toggle, 6, 0, 0, 70, 30, 30");
	addPanelWidget("momentary, momentary, 6, 0, 40, 70, 30, 30");
	addPanelWidget("radio, radio, 6, 0, 80, 70, 30, 30");

	end();
}

void PanelManager::switchChangeCallback(Fl_Widget* w)
{
	auto it = std::find(m_widgets.begin(), m_widgets.end(), w);

	if (it != m_widgets.end()) {
		int index = it - m_widgets.begin();
		
		Switches* s = static_cast<Switches*>(w);
				
		std::cout << "PanelWidget " << index << " Value changed to " << std::to_string(s->getValue()) << std::endl;
	}
}

void PanelManager::addPanelWidget(std::string const& widgetParams)
{
	std::vector<std::string> args;

	std::istringstream ss(widgetParams);
	std::string token;
	while(std::getline(ss, token, ',')) {
		args.push_back(token);
	}
	
	if (args[0] == "toggle") {
		if (args.size() == 8) {
			ToggleSwitches* ts = new ToggleSwitches(args[1], std::stoi(args[2]), std::stoi(args[3]), std::stoi(args[4]), std::stoi(args[5]), std::stoi(args[6]), std::stoi(args[7]));
			ts->setValueChangeCallback(&staticSwitchChangeCallback);
			m_widgets.push_back(ts);
		} else {

		}
	} else if (args[0] == "momentary") {
		if (args.size() == 8) {
			MomentarySwitches* ms = new MomentarySwitches(args[1], std::stoi(args[2]), std::stoi(args[3]), std::stoi(args[4]), std::stoi(args[5]), std::stoi(args[6]), std::stoi(args[7]));
			ms->setValueChangeCallback(&staticSwitchChangeCallback);
			m_widgets.push_back(ms);
		} else {

		}
	} else if (args[0] == "radio") {
		if (args.size() == 8) {
			RadioSwitches* ms = new RadioSwitches(args[1], std::stoi(args[2]), std::stoi(args[3]), std::stoi(args[4]), std::stoi(args[5]), std::stoi(args[6]), std::stoi(args[7]));
			ms->setValueChangeCallback(&staticSwitchChangeCallback);
			m_widgets.push_back(ms);
		} else {

		}
	}
}

ToggleSwitches::ToggleSwitches(std::string const& name, int count, int x, int y, int labelWidth, int buttonWidth, int height) :
	Switches(x, y, labelWidth + (count * buttonWidth), height)
{
	m_label = new Fl_Box(x, y, labelWidth, height, "");
	m_label->copy_label(name.c_str());

	int buttonX = x + labelWidth;
	for (char i = 0; i < count; i++) {
	    
		Fl_Toggle_Button* button = new Fl_Toggle_Button(buttonX, y, buttonWidth, height, "");
		button->copy_label(std::to_string(i).c_str());
		button->when(FL_WHEN_RELEASE);
		button->callback(staticSwitchChangeCallback, (void*)this);
		m_buttons.push_back(button);
		buttonX += buttonWidth;
	}
	
	end();
}

MomentarySwitches::MomentarySwitches(std::string const& name, int count, int x, int y, int labelWidth, int buttonWidth, int height) :
	Switches(x, y, labelWidth + (count * buttonWidth), height)
{
	m_label = new Fl_Box(x, y, labelWidth, height, "");
	m_label->copy_label(name.c_str());

	int buttonX = x + labelWidth;
	for (char i = 0; i < count; i++) {
	    
		Fl_Button* button = new Fl_Button(buttonX, y, buttonWidth, height, "");
		button->copy_label(std::to_string(i).c_str());
		button->when(FL_WHEN_CHANGED);
		button->callback(staticSwitchChangeCallback, (void*)this);
		m_buttons.push_back(button);
		buttonX += buttonWidth;
	}
	
	end();
}

RadioSwitches::RadioSwitches(std::string const& name, int count, int x, int y, int labelWidth, int buttonWidth, int height) :
	Switches(x, y, labelWidth + (count * buttonWidth), height)
{
	m_label = new Fl_Box(x, y, labelWidth, height, "");
	m_label->copy_label(name.c_str());

	int buttonX = x + labelWidth;
	for (char i = 0; i < count; i++) {
	    
		Fl_Button* button = new Fl_Toggle_Button(buttonX, y, buttonWidth, height, "");
		button->copy_label(std::to_string(i).c_str());
		button->type(FL_RADIO_BUTTON);
		button->when(FL_WHEN_RELEASE);
		button->callback(staticSwitchChangeCallback, (void*)this);
		m_buttons.push_back(button);
		buttonX += buttonWidth;
	}
	
	end();

	m_buttons[0]->setonly();
}

void RadioSwitches::switchChangeCallback(Fl_Widget* w)
{
	Fl_Button* button = (Fl_Button*)w;
	auto it = std::find(m_buttons.begin(), m_buttons.end(), button);

	if (it != m_buttons.end()) {
		for (int i = 0; i < m_buttons.size(); i ++) {
			if (m_buttons[i]->value()) {
				m_value |= (1ul << i);
			} else {
				m_value &= ~(1ul << i);
			}
		}

		std::cout << "ToggleSwitches value is " << std::to_string(m_value) << std::endl;

		if (m_valueChangeCallback != 0) {
			m_valueChangeCallback(this, (void*)parent());
		}
	}
}
