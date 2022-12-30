#include <iostream>
#include <sstream>
#include <algorithm>
#include <string>

#include "fp_components.h"

#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>
#include <FL/fl_ask.H>

void Switches::switchChangeCallback(Fl_Widget* w)
{
	if (w == NULL) {		
		for (int i = 0; i < m_buttons.size(); i ++) {
			if (static_cast<Fl_Button*>(m_buttons[i])->value()) {
				m_value |= (1ul << i);
			} else {
				m_value &= ~(1ul << i);
			}
		}
	} else {
		Fl_Button* button = (Fl_Button*)w;
		auto it = std::find(m_buttons.begin(), m_buttons.end(), button);
		int index = it - m_buttons.begin();
		unsigned long bit = button->value();
		
		if (bit) {
			m_value |= (1ul << index);
		} else {
			m_value &= ~(1ul << index);
		}
	}

	//~std::cout << "Switches value is now " << std::to_string(m_value) << std::endl;

	if (m_valueChangeCallback != 0) {
		m_valueChangeCallback(this, static_cast<void*>(parent()));
	}
}

PanelManager::PanelManager(std::string const& name, int width, int height, std::vector<std::string> const& panelConfig, bool echo) :
	Fl_Window(width, height),
	m_echo(echo)
{
	m_panelOpen = false;
	
	copy_label(name.c_str());
	
	for (const auto& widgetConfig : panelConfig) {
		addPanelWidget(widgetConfig);
	}

	callback(staticPromptedHideCallback, this);

	end();

	for (int i = 0; i < m_switchesWidgets.size(); i++) {
		auto ptr = dynamic_cast<Switches*>(m_switchesWidgets[i]);
		if (ptr != NULL) {
			ptr->switchChangeCallback(NULL);
		}
	}
}

void PanelManager::promptedHideCallback()
{
	switch (fl_choice("Close the front panel?", "Yes", "No", 0) ) {
	case 0:		// Yes
		hide();
		m_panelOpen = false;
		break;
	case 1:		// No (default)
		break;
	}
}

void PanelManager::requestShowCallback()
{
	m_panelOpen = true;
	show();
}

void PanelManager::requestHideCallback()
{
	hide();
	m_panelOpen = false;
}

void PanelManager::open()
{
	Fl::awake(staticRequestShowCallback, this);
}

void PanelManager::close()
{
	Fl::awake(staticRequestHideCallback, this);
}

std::atomic_bool const& PanelManager::isOpen() const
{
	return m_panelOpen;
}

void PanelManager::switchChangeCallback(Fl_Widget* w)
{
	auto it = std::find(m_switchesWidgets.begin(), m_switchesWidgets.end(), w);

	if (it != m_switchesWidgets.end()) {

		int index = it - m_switchesWidgets.begin();
		Switches* s = static_cast<Switches*>(w);
		unsigned long value = s->getValue();
				
		if (m_echo) {
			std::cout << "SwitchesWidget " << index << " Value changed to " << std::to_string(value) << std::endl;
		}

		switchesChanged(index, value);
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
			m_switchesWidgets.push_back(ts);
			m_switchesWidgetValues.push_back(0ul);
		} else {

		}
	} else if (args[0] == "momentary") {
		if (args.size() == 8) {
			MomentarySwitches* ms = new MomentarySwitches(args[1], std::stoi(args[2]), std::stoi(args[3]), std::stoi(args[4]), std::stoi(args[5]), std::stoi(args[6]), std::stoi(args[7]));
			ms->setValueChangeCallback(&staticSwitchChangeCallback);
			m_switchesWidgets.push_back(ms);
			m_switchesWidgetValues.push_back(0ul);
		} else {

		}
	} else if (args[0] == "radio") {
		if (args.size() == 9) {
			
			std::vector<std::string> nameArgs;
			std::istringstream ss(args[3]);
			std::string nameToken;
			while(std::getline(ss, nameToken, '.')) {
				nameToken.erase(remove_if(nameToken.begin(), nameToken.end(), isspace), nameToken.end());
				nameArgs.push_back(nameToken);
			}
			
			RadioSwitches* ms = new RadioSwitches(args[1], std::stoi(args[2]), nameArgs, std::stoi(args[4]), std::stoi(args[5]), std::stoi(args[6]), std::stoi(args[7]), std::stoi(args[8]));
			ms->setValueChangeCallback(&staticSwitchChangeCallback);
			m_switchesWidgets.push_back(ms);
			m_switchesWidgetValues.push_back(0ul);
		} else {

		}
	} else if (args[0] == "indicator") {
		if (args.size() == 8) {
			BasicIndicators* bi = new BasicIndicators(args[1], std::stoi(args[2]), std::stoi(args[3]), std::stoi(args[4]), std::stoi(args[5]), std::stoi(args[6]), std::stoi(args[7]));
			m_indicatorsWidgets.push_back(bi);
		} else {

		}
	}
}

bool PanelManager::haveSwitchesChanged() const
{
	return m_switchesHaveChanged;
}

std::vector<int> PanelManager::getChangedSwitchesIndices()
{
	std::vector<int> changedSwitches;
	std::swap(m_changedSwitches, changedSwitches);
	m_switchesHaveChanged = false;
	return changedSwitches;
}

unsigned long PanelManager::getChangedSwitchesValue(int widgetIndex) const
{
	return m_switchesWidgetValues[widgetIndex];
}

void PanelManager::switchesChanged(int widgetIndex, unsigned long value)
{
	std::unique_lock<std::mutex> lock (m_valueChangeMutex);
	m_switchesHaveChanged = true;
	m_changedSwitches.push_back(widgetIndex);
	m_switchesWidgetValues[widgetIndex] = value;
}

void PanelManager::setIndicatorsValue(int indicatorsIndex, unsigned long value)
{
	Fl::lock();

	static_cast<Indicators*>(m_indicatorsWidgets[indicatorsIndex])->setValue(value);
	
	Fl::unlock();
}

BasicIndicators::BasicIndicators(std::string const& name, int count, int x, int y, int labelWidth, int indicatorWidth, int height) :
	Indicators(x, y, labelWidth + (count * indicatorWidth), height)
{
	m_label = new Fl_Box(x, y, labelWidth, height, "");
	m_label->copy_label(name.c_str());

	int indicatorX = x + labelWidth;
	for (char i = 0; i < count; i++) {
		Fl_Box* box = new Fl_Box(indicatorX, y, indicatorWidth, height, "0");
		m_indicators.push_back(box);
		indicatorX += indicatorWidth;
	}
	
	end();
}

void BasicIndicators::setValue(unsigned long value)
{	
	for (int i = 0; i < m_indicators.size(); i++) {
		bool m_value_bit = (m_value >> i) & 1ul;
		bool value_bit = (value >> i) & 1ul;
		if (m_value_bit ^ value_bit) {	// Different
			if (value_bit) {
				m_indicators[i]->copy_label("1");
			} else {
				m_indicators[i]->copy_label("0");
			}
		}
	}
	m_value = value;
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
		button->callback(staticSwitchChangeCallback, static_cast<void*>(this));
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
		button->callback(staticSwitchChangeCallback, static_cast<void*>(this));
		m_buttons.push_back(button);
		buttonX += buttonWidth;
	}
	
	end();
}

RadioSwitches::RadioSwitches(std::string const& name, int count, std::vector<std::string> const& names, int x, int y, int labelWidth, int buttonWidth, int height) :
	Switches(x, y, labelWidth + (count * buttonWidth), height)
{
	m_label = new Fl_Box(x, y, labelWidth, height, "");
	m_label->copy_label(name.c_str());

	int buttonX = x + labelWidth;
	for (char i = 0; i < count; i++) {
	    
		Fl_Button* button = new Fl_Toggle_Button(buttonX, y, buttonWidth, height, "");
		button->copy_label(names[i].c_str());
		button->type(FL_RADIO_BUTTON);
		button->when(FL_WHEN_RELEASE);
		button->callback(staticSwitchChangeCallback, static_cast<void*>(this));
		m_buttons.push_back(button);
		buttonX += buttonWidth;
	}
	
	end();

	static_cast<Fl_Button*>(m_buttons[0])->setonly();
}

void RadioSwitches::switchChangeCallback(Fl_Widget* w)
{
	for (int i = 0; i < m_buttons.size(); i ++) {
		if (static_cast<Fl_Button*>(m_buttons[i])->value()) {
			m_value |= (1ul << i);
		} else {
			m_value &= ~(1ul << i);
		}
	}

	//~std::cout << "Switches value is now " << std::to_string(m_value) << std::endl;

	if (m_valueChangeCallback != NULL) {
		m_valueChangeCallback(this, static_cast<void*>(parent()));
	}
}
