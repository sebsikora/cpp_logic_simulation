#ifndef PANEL_DISPLAY_H
#define PANEL_DISPLAY_H

#include <string>

struct bulb_chars {
	char false_char;
	char true_char;
};

class Display {
	public:
		Display(WINDOW* win, std::string const& name, int x, int y, int width, int charwidth, bulb_chars const& bulbs = {'F', 'T'});
		void Set(int value, bool refresh_now);
		void Draw(bool refresh_now);
		void MoveDisplay(int oset_y, bool refresh_now);
	private:
		void PrintName();
		void Update(bool refresh_now);

		WINDOW* m_win;
		std::string m_name;
		bulb_chars m_bulbs;
		int m_x;
		int m_y;
		int m_width;
		int m_charwidth;
		int m_value = 0;
};

#endif
