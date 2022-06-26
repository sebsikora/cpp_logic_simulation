#include <string>

#include <ncurses.h>

#include "display.h"

Display::Display(WINDOW* win, std::string const& name, int x, int y, int width, int charwidth, bulb_chars const& bulbs) {
	m_win = win;
	m_name = name;
	m_bulbs = bulbs;
	m_x = x;
	m_y = y;
	m_width = width;
	m_charwidth = charwidth;
	Draw(true);
}

void Display::Draw(bool refresh_now) {
	PrintName();
	Update(refresh_now);
}

void Display::PrintName() {
	wmove(m_win, m_y, m_x);
	wprintw(m_win, m_name.substr(0, m_charwidth).c_str());
}

void Display::Update(bool refresh_now) {
	int x = m_x;
	int y = m_y + 1;
	for (int bit_pos = m_width - 1; bit_pos >= 0; bit_pos --) {
		int masked_value = ((1 << bit_pos) & m_value) >> bit_pos;
		if (masked_value) {
			mvwaddch(m_win, y, x, m_bulbs.true_char);
		} else {
			mvwaddch(m_win, y, x, m_bulbs.false_char);
		}
		x = x + 2;
	}
	if (refresh_now) {
		wrefresh(m_win);
	}
}

void Display::Set(int value, bool refresh_now) {
	m_value = value;
	Update(refresh_now);
}

void Display::MoveDisplay(int oset_y, bool refresh_now) {
	m_y = m_y + oset_y;
	Draw(refresh_now);
}
