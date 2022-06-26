#ifndef PANEL_PMGR_H
#define PANEL_PMGR_H

#include <string>
#include <vector>
#include <memory>
#include <ncurses.h>

#include "panel.h"

class PanelManager {
	public:
		PanelManager(WINDOW* win, std::string const& name);
		void Draw(bool refresh_now);
		void Build(std::string const& recipe);
		void AddPanel(std::string const& name, int columns);
		bool AddDisplay(int panel_id, int column_id, std::string const& name, int width, bulb_chars bulbs = {'F', 'T'});
		bool AddSwitch(int panel_id, int column_id, std::string const& switch_text, bool momentary_flag = false, bool default_state = false);
		bool AddSwitchBank(int panel_id, int column_id, std::string const& switch_text, int width, bool momentary_flag = false, bool default_state = false);
		bool SetDisplayValue(int panel_id, int display_id, int value, bool refresh_now);
		bool CheckInput(bool refresh_now);
	private:
		void BuildInstructionError(std::string const& current_instruction);
		WINDOW* m_win;
		std::string m_name;
		int m_max_y;
		std::vector<std::unique_ptr<Panel>> m_panels = {};
		bool m_has_switches = false;
};

#endif
