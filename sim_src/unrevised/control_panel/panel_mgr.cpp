#ifndef CTRL
#define CTRL(c) ((c) & 037)
#endif

#include <string>
#include <vector>
#include <memory>

#include <ncurses.h>

#include "panel_mgr.h"

PanelManager::PanelManager(WINDOW* win, std::string const& name) {
	m_win = win;
	m_name = name;
	m_max_y = 1;
	// curses settings -----
	nodelay(m_win, true);
	noecho();
	cbreak();
	curs_set(0);
	keypad(m_win, true);
	// ---------------------
	Draw(true);
}

void PanelManager::Draw(bool refresh_now) {
	wclear(m_win);
	box(m_win, 0, 0);
	wmove(m_win, 0, 9);
	wprintw(m_win, (" " + m_name + " ").c_str());
	if (refresh_now) {
		wrefresh(m_win);
	}
}

void PanelManager::AddPanel(std::string const& name, int columns) {
	m_panels.push_back(std::make_unique<Panel>(Panel(m_win, name, m_max_y, columns)));
	m_max_y += 1;
	Draw(false);
	for (const auto& panel : m_panels) {
		panel.get()->Draw(false);
	}
	wrefresh(m_win);
}

bool PanelManager::AddDisplay(int panel_id, int column_id, std::string const& name, int width, bulb_chars bulbs) {
	if (panel_id < m_panels.size()) {
		int new_y_offset = m_panels[panel_id].get()->NewDisplay(name, width, column_id, bulbs);
		if (new_y_offset > 0) {
			m_max_y += new_y_offset;
			Draw(false);
			for (int id = 0; id < panel_id + 1; id ++) {
				m_panels[id].get()->MovePanel(0, false);
			}
			for (int id = panel_id + 1; id < m_panels.size(); id ++) {
				m_panels[id].get()->MovePanel(new_y_offset, false);
			}
			wrefresh(m_win);
		}
		return true;
	} else {
		return false;
	}
}

bool PanelManager::AddSwitch(int panel_id, int column_id, std::string const& switch_text, bool momentary_flag, bool default_state) {
	if (panel_id < m_panels.size()) {
		int new_y_offset = m_panels[panel_id].get()->NewSwitch(switch_text, column_id, momentary_flag, default_state);
		if (new_y_offset > 0) {
			m_max_y += new_y_offset;
			Draw(false);
			for (int id = 0; id < panel_id + 1; id ++) {
				m_panels[id].get()->MovePanel(0, false);
			}
			for (int id = panel_id + 1; id < m_panels.size(); id ++) {
				m_panels[id].get()->MovePanel(new_y_offset, false);
			}
			wrefresh(m_win);
		}
		if (!m_has_switches) {
			m_has_switches = true;
			mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
		}
		return true;
	} else {
		return false;
	}
}

bool PanelManager::AddSwitchBank(int panel_id, int column_id, std::string const& switch_text, int width, bool momentary_flag, bool default_state) {
	if (panel_id < m_panels.size()) {
		int new_y_offset = m_panels[panel_id].get()->NewSwitchBank(switch_text, width, column_id, momentary_flag, default_state);
		if (new_y_offset > 0) {
			m_max_y += new_y_offset;
			Draw(false);
			for (int id = 0; id < panel_id + 1; id ++) {
				m_panels[id].get()->MovePanel(0, false);
			}
			for (int id = panel_id + 1; id < m_panels.size(); id ++) {
				m_panels[id].get()->MovePanel(new_y_offset, false);
			}
			wrefresh(m_win);
		}
		if (!m_has_switches) {
			m_has_switches = true;
			mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
		}
		return true;
	} else {
		return false;
	}
}

bool PanelManager::SetDisplayValue(int panel_id, int display_id, int value, bool refresh_now) {
	if (panel_id < m_panels.size()) {
		bool success = m_panels[panel_id].get()->SetDisplayValue(display_id, value, refresh_now);
		return success;
	} else {
		return false;
	}
}

bool PanelManager::CheckInput(bool refresh_now) {
	bool val = false;
	int c = wgetch(m_win);
	if (c == KEY_MOUSE) {
		MEVENT event;
		if(getmouse(&event) == OK) {
			int click_type = 0;
			if (event.bstate & BUTTON1_CLICKED) {
				click_type = 1;
			} else if (event.bstate & BUTTON1_PRESSED) {
				click_type = 2;
			} else if (event.bstate & BUTTON1_RELEASED) {
				click_type = 3;
			}
			if (click_type != 0) {
				int x = event.x;
				int y = event.y;
				for (const auto& panel_unique_ptr : m_panels) {
					panel_unique_ptr.get()->CheckSwitchClick(x, y, click_type, refresh_now);
				}
			}
		}
	} else
	if (c == 005) {		// CTRL + E
		val = true;
	}
	return val;
}

void PanelManager::Build(std::string const& recipe) {
	std::string working_recipe = recipe;
	while (true) {
		// Capture substring upto next newline character.
		size_t position_of_next_newline = working_recipe.find('\n');
		bool newline_found = false;
		if (position_of_next_newline != std::string::npos) {
			newline_found = true;
		}
		if (newline_found) {
			std::string current_instruction = working_recipe.substr(0, position_of_next_newline);
			std::string working_instruction = current_instruction;
			std::vector<std::string> instruction_fields = {};
			// Split out instruction and operands.
			while (true) {
				size_t position_of_next_comma = working_instruction.find(',');
				std::string field = working_instruction.substr(0, position_of_next_comma);
				instruction_fields.push_back(field);
				working_instruction = working_instruction.substr(position_of_next_comma + 1, std::string::npos);
				if (position_of_next_comma == std::string::npos) {
					break;
				}
			}
			// Decode instruction.
			if ((instruction_fields[0] == "AP") && (instruction_fields.size() == 3)) {
				// (AP) Add Panel.
				std::string name = instruction_fields[1];
				int columns = std::stoi(instruction_fields[2]);
				AddPanel(name, columns);
			} else if ((instruction_fields[0] == "AD") && (instruction_fields.size() == 5)) {
				// (AD) Add Display.
				int panel_id = std::stoi(instruction_fields[1]);
				int column_id = std::stoi(instruction_fields[2]);
				std::string name = instruction_fields[3];
				int width = std::stoi(instruction_fields[4]);
				AddDisplay(panel_id, column_id, name, width);
			} else if ((instruction_fields[0] == "AS") && (instruction_fields.size() == 6)) {
				// (AS) Add Switch.
				int panel_id = std::stoi(instruction_fields[1]);
				int column_id = std::stoi(instruction_fields[2]);
				std::string switch_text = instruction_fields[3];
				bool momentary_flag;
				if (instruction_fields[4] == "true") {
					momentary_flag = true;
				} else {
					momentary_flag = false;
				}
				bool default_state;
				if (instruction_fields[5] == "true") {
					default_state = true;
				} else {
					default_state = false;
				}
				AddSwitch(panel_id, column_id, switch_text, momentary_flag, default_state);
			} else if ((instruction_fields[0] == "AB") && (instruction_fields.size() == 7)) {
				// (AB) Add Switch Bank.
				int panel_id = std::stoi(instruction_fields[1]);
				int column_id = std::stoi(instruction_fields[2]);
				std::string switch_text = instruction_fields[3];
				int width = std::stoi(instruction_fields[4]);
				bool momentary_flag;
				if (instruction_fields[5] == "true") {
					momentary_flag = true;
				} else {
					momentary_flag = false;
				}
				bool default_state;
				if (instruction_fields[6] == "true") {
					default_state = true;
				} else {
					default_state = false;
				}
				AddSwitchBank(panel_id, column_id, switch_text, width, momentary_flag, default_state);
			} else {
				// Can't decode instruction.
				BuildInstructionError(current_instruction);
			}
			working_recipe = working_recipe.substr(position_of_next_newline + 1, std::string::npos);
		} else {
			break;
		}
	}
}

void PanelManager::BuildInstructionError(std::string const& current_instruction) {
	int width = 60;
	int height = 6;
	std::string error_message = "Can't parse build instruction " + current_instruction ;
	error_message = error_message.substr(0, width - 4);
	int oset_x = (getmaxx(stdscr) - width) / 2;
	int oset_y = ((getmaxy(stdscr) - height) / 2);
	WINDOW* about_window = newwin(height, width, oset_y, oset_x);
	box(about_window, 0, 0);
	wmove(about_window, 0, 5);
	wprintw(about_window, " HELP! ");
	wmove(about_window, 2, 2);
	wprintw(about_window, error_message.c_str());
	wgetch(about_window); //also invokes wrefresh()
}
