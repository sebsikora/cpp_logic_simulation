#include <string>

#include <ncurses.h>

#include "switch.h"

Switch::Switch(WINDOW* win, std::string const& switch_text, bool momentary_flag, bool default_state, int x, int y, int charwidth) {
	m_win = win;
	m_switch_text = switch_text;
	m_x = x;
	m_y = y + 1;
	m_charwidth = charwidth;
	m_momentary_flag = momentary_flag;
	m_state = default_state;
	Draw(true);
}

void Switch::Draw(bool refresh_now) {
	wmove(m_win, m_y, m_x);
	std::string switch_text_now;
	if (m_state) {
		switch_text_now = m_switch_text.substr(0, m_charwidth - 2) + " T";
	} else {
		switch_text_now = m_switch_text.substr(0, m_charwidth - 2) + " F";
	}
	wprintw(m_win, switch_text_now.substr(0, m_charwidth).c_str());
	if (refresh_now) {
		wrefresh(m_win);
	}
}

bool Switch::CheckSwitchClick(int x, int y, int click_type, bool refresh_now) {
	int switch_width = m_switch_text.size();
	if (switch_width > m_charwidth) {
		switch_width = m_charwidth;
	}
	if ((x >= m_x) && (x < (m_x + switch_width)) && (y == m_y)) {
		if ((!m_momentary_flag) && ((click_type == 1) || (click_type == 2))) {
			Activate(refresh_now);
		} else if ((m_momentary_flag) && ((click_type == 2) || (click_type == 3))) {
			Activate(refresh_now);
		} else if ((m_momentary_flag) && (click_type == 1)) {
			// Do nothing.
		}
		return true;
	} else {
		return false;
	}
}

void Switch::Activate(bool refresh_now) {
	m_state = !m_state;
	Draw(refresh_now);
}

void Switch::MoveSwitch(int oset_y, bool refresh_now) {
	m_y = m_y + oset_y;
	Draw(refresh_now);
}
