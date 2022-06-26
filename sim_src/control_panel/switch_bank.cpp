#include <string>

#include <ncurses.h>

#include "switch_bank.h"

SwitchBank::SwitchBank(WINDOW* win, std::string const& switch_text, int width, bool momentary_flag, bool default_state, int x, int y, int charwidth) {
	m_win = win;
	m_switch_text = switch_text;
	m_width = width;
	m_x = x;
	m_y = y;
	m_charwidth = charwidth;
	m_momentary_flag = momentary_flag;
	m_states = std::vector<bool>(m_width, default_state);
	Draw(true);
}

void SwitchBank::Draw(bool refresh_now) {
	wmove(m_win, m_y, m_x);
	wprintw(m_win, m_switch_text.substr(0, m_charwidth).c_str());
	int switch_x_pos = m_x, switch_y_pos = m_y + 1;
	for (int switch_index = (m_width - 1); switch_index >= 0; switch_index --) {
		DrawSwitch(switch_index, false);
		switch_x_pos += 2;
	}
	if (refresh_now) {
		wrefresh(m_win);
	}
}

void SwitchBank::DrawSwitch(int switch_index, bool refresh_now) {
	int min_x = m_x + (2 * (m_width - 1)) - (switch_index * 2);
	int min_y = m_y + 1;
	int max_x = min_x + 1;
	int max_y = m_y + 2;
	char state_char;
	if (m_states[switch_index]) {
		state_char = 'T';
	} else {
		state_char = 'F';
	}
	mvwaddch(m_win, min_y, min_x, state_char);
	mvwaddch(m_win, min_y, max_x, ACS_URCORNER);
	mvwaddch(m_win, max_y, max_x, ACS_LRCORNER);
	mvwaddch(m_win, max_y, min_x, ACS_LLCORNER);
	if (refresh_now) {
		wrefresh(m_win);
	}
}

bool SwitchBank::CheckSwitchBankClick(int click_x_pos, int click_y_pos, int click_type, bool refresh_now) {
	int min_x = m_x;
	int min_y = m_y + 1;
	int max_x = m_x + (2 * m_width);
	int max_y = min_y + 2;
	if (((click_x_pos >= min_x) && (click_x_pos < max_x)) && ((click_y_pos >= min_y) && (click_y_pos < max_y))) {
		for (int switch_index = (m_width - 1); switch_index >= 0; switch_index --) {
			if(CheckSwitchClick(switch_index, click_x_pos, click_y_pos, click_type, refresh_now)) {
				return true;
			}
		}
		return false;
	}
	return false;
}

bool SwitchBank::CheckSwitchClick(int switch_index, int click_x_pos, int click_y_pos, int click_type, bool refresh_now) {
	int min_x = m_x + (2 * (m_width - 1)) - (switch_index * 2);
	int min_y = m_y + 1;
	int max_x = min_x + 2;
	int max_y = min_y + 2;
	if (((click_x_pos >= min_x) && (click_x_pos < max_x)) && ((click_y_pos >= min_y) && (click_y_pos < max_y))) {
		if ((!m_momentary_flag) && ((click_type == 1) || (click_type == 2))) {
			Activate(switch_index, refresh_now);
		} else if ((m_momentary_flag) && ((click_type == 2) || (click_type == 3))) {
			Activate(switch_index, refresh_now);
		} else if ((m_momentary_flag) && (click_type == 1)) {
			// Do nothing.
		} else {
			// Do nothing.
		}
		return true;
	} else {
		return false;
	}
}

void SwitchBank::Activate(int switch_index, bool refresh_now) {
	m_states[switch_index] = !m_states[switch_index];
	DrawSwitch(switch_index, refresh_now);
}

void SwitchBank::MoveSwitchBank(int oset_y, bool refresh_now) {
	m_y = m_y + oset_y;
	Draw(refresh_now);
}

