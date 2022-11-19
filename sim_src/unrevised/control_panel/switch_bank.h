#ifndef PANEL_SW_BANK_H
#define PANEL_SW_BANK_H

#include <string>
#include <vector>

class SwitchBank {
	public:
		SwitchBank(WINDOW* win, std::string const& switch_text, int width, bool momentary_flag, bool default_state, int x, int y, int charwidth);
		void Draw(bool refresh_now);
		void MoveSwitchBank(int oset_y, bool refresh_now);
		bool CheckSwitchBankClick(int click_x_pos, int click_y_pos, int click_type, bool refresh_now);
	private:
		void DrawSwitch(int switch_index, bool refresh_now);
		bool CheckSwitchClick(int switch_index, int click_x_pos, int click_y_pos, int click_type, bool refresh_now);
		void Activate(int switch_index, bool refresh_now);
		WINDOW* m_win;
		std::string m_switch_text;
		int m_width;
		int m_x;
		int m_y;
		int m_charwidth;
		bool m_momentary_flag;
		std::vector<bool> m_states;
};

#endif

