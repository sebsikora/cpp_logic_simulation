#include <string>
#include <vector>
#include <memory>

#include <ncurses.h>

#include "display.h"
#include "switch.h"
#include "switch_bank.h"
#include "panel.h"

Panel::Panel(WINDOW* win, std::string const& name, int base_y, int columns) {
	m_win = win;
	m_name = name;
	m_columns = columns;
	m_base_y = base_y;
	
    int max_y;
    int max_x;
    getmaxyx(m_win, max_y, max_x);
    m_max_x = max_x;
    int column_x_inc = (int)((m_max_x - 4) / columns);
	
	int x = 2;
	int y = base_y + 1;
	for (int i = 0; i < m_columns; i ++) {
		m_col_oset_x.push_back(x);
		m_col_oset_y.push_back(y);
		x += column_x_inc;
	}
	m_max_oset_y = y;
	m_column_charwidth = column_x_inc - 2;
}

int Panel::NewDisplay(std::string const& name, int width, int column, bulb_chars const& bulbs) {
	if (column < m_col_oset_x.size()) {
		m_displays.emplace_back(std::make_unique<Display>(Display(m_win, name, m_col_oset_x[column], m_col_oset_y[column], width, m_column_charwidth, bulbs)));
		m_col_oset_y[column] += 3;
		if (m_col_oset_y[column] > m_max_oset_y) {
			m_max_oset_y = m_col_oset_y[column];
			return 3;
		} else {
			return 0;
		}
	} else {
		return 0;
	}
}

int Panel::NewSwitch(std::string const& switch_text, int column, bool momentary_flag, bool default_state) {
	if (column < m_col_oset_x.size()) {
		m_switches.emplace_back(std::make_unique<Switch>(Switch(m_win, switch_text, momentary_flag, default_state, m_col_oset_x[column], m_col_oset_y[column], m_column_charwidth)));
		m_col_oset_y[column] += 3;
		if (!m_has_switches) {
			m_has_switches = true;
		}
		if (m_col_oset_y[column] > m_max_oset_y) {
			m_max_oset_y = m_col_oset_y[column];
			return 3;
		} else {
			return 0;
		}
	} else {
		return 0;
	}
}

int Panel::NewSwitchBank(std::string const& switch_text, int width, int column, bool momentary_flag, bool default_state) {
	if (column < m_col_oset_x.size()) {
		m_switch_banks.emplace_back(std::make_unique<SwitchBank>(SwitchBank(m_win, switch_text, width, momentary_flag, default_state, m_col_oset_x[column], m_col_oset_y[column], m_column_charwidth)));
		m_col_oset_y[column] += 3;
		if (!m_has_switches) {
			m_has_switches = true;
		}
		if (m_col_oset_y[column] > m_max_oset_y) {
			m_max_oset_y = m_col_oset_y[column];
			return 3;
		} else {
			return 0;
		}
	} else {
		return 0;
	}
}

bool Panel::SetDisplayValue(int display_index, int value, bool refresh_now) {
	if (display_index < m_displays.size()) {
		m_displays[display_index].get()->Set(value, refresh_now);
		return true;
	} else {
		return false;
	}
}

bool Panel::CheckSwitchClick(int x, int y, int click_type, bool refresh_now) {
	if (m_has_switches) {
		// Only check child switches if click falls within panel.
		if  (((x >= 0) && (x < m_max_x)) && ((y >= m_base_y) && (y < m_max_oset_y))) {
			for (const auto& switch_unique_ptr : m_switches) {
				if(switch_unique_ptr.get()->CheckSwitchClick(x, y, click_type, refresh_now)) {
					return true;
				}
			}
			for (const auto& switchbank_unique_ptr : m_switch_banks) {
				if(switchbank_unique_ptr.get()->CheckSwitchBankClick(x, y, click_type, refresh_now)) {
					return true;
				}
			}
			return false;
		} else {
			return false;
		}
	} else {
		return false;
	}
}

void Panel::MovePanel(int oset_y, bool refresh_now) {
	m_base_y += oset_y;
	m_max_oset_y += oset_y;
	for (auto& entry : m_col_oset_y) {
		entry += oset_y;
	}
	Draw(false);
	for (const auto& display_unique_ptr : m_displays) {
		display_unique_ptr.get()->MoveDisplay(oset_y, false);
	}
	for (const auto& switch_unique_ptr : m_switches) {
		switch_unique_ptr.get()->MoveSwitch(oset_y, false);
	}
	for (const auto& switchbank_unique_ptr : m_switch_banks) {
		switchbank_unique_ptr.get()->MoveSwitchBank(oset_y, false);
	}
	if (refresh_now) {
		wrefresh(m_win);
	}
}

void Panel::Draw(bool refresh_now) {
	std::string short_name = m_name.substr(0, 20);
	int short_name_x_pos = 9;
	wmove(m_win, m_base_y, 0);
	waddch(m_win, ACS_LTEE);
	for (int i = 1; i < (m_max_x - 1); i ++) {
		waddch(m_win, ACS_HLINE);
	}
	waddch(m_win, ACS_RTEE);
	wmove(m_win, m_base_y, short_name_x_pos);
	wprintw(m_win, (" " + short_name + " ").c_str());
	if (refresh_now) {
		wrefresh(m_win);
	}
}
