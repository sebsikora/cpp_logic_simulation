#ifndef PANEL_SWITCH_H
#define PANEL_SWITCH_H

#include <string>

class Switch {
	public:
		Switch(WINDOW* win, std::string const& switch_text, bool momentary_flag, bool default_state, int x, int y, int charwidth);
		void Draw(bool refresh_now);
		bool CheckSwitchClick(int x, int y, int click_type, bool refresh_now);
		void MoveSwitch(int oset_y, bool refresh_now);
		
	private:
		void Activate(bool refresh_now);
		WINDOW* m_win;
		std::string m_switch_text;
		int m_x;
		int m_y;
		int m_charwidth;
		bool m_momentary_flag;
		bool m_state;
};

#endif
