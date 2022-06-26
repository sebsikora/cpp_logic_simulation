#ifndef PANEL_PANEL_H
#define PANEL_PANEL_H

#include <string>
#include <vector>
#include <memory>
#include <ncurses.h>

#include "display.h"
#include "switch.h"
#include "switch_bank.h"

class Panel {
	public:
		Panel(WINDOW* win, std::string const& name, int base_y = 0, int columns = 1);
		
		int NewDisplay(std::string const& name, int width, int column = 0, bulb_chars const& bulbs = {'F', 'T'});
		int NewSwitch(std::string const& switch_text, int column = 0, bool momentary_flag = false, bool default_state = false);
		int NewSwitchBank(std::string const& switch_text, int width, int column = 0, bool momentary_flag = false, bool default_state = false);
		
		bool SetDisplayValue(int display_index, int value, bool refresh_now);
		bool CheckSwitchClick(int x, int y, int click_type, bool refresh_now);

		void MovePanel(int oset_y, bool refresh_now);
		void Draw(bool refresh_now);
		
	private:
		std::vector<std::unique_ptr<Display>> m_displays = {};
		std::vector<std::unique_ptr<Switch>> m_switches = {};
		std::vector<std::unique_ptr<SwitchBank>> m_switch_banks = {};
		WINDOW* m_win;
		std::string m_name;
		int m_columns;
		int m_column_charwidth;
		int m_base_y;
		int m_max_x;
		int m_max_oset_y;
		std::vector<int> m_col_oset_x = {};
		std::vector<int> m_col_oset_y = {};
		bool m_has_switches = false;
		
};

#endif
