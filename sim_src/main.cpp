#include <string>

#include <ncurses.h>
#include <unistd.h>

#include "panel_mgr.h"

int main()
{	
	WINDOW* mw = initscr();			/* Start curses mode 		  */
	int max_y;
    int max_x;
    getmaxyx(mw, max_y, max_x);

	WINDOW* mainwin;
	mainwin = newwin(max_y, max_x, 0, 0);
	PanelManager pm(mainwin, "test_panel");
	
	//~pm.AddPanel("panel_one", 3);
	//~pm.AddPanel("panel_two", 2);
	std::string build_string("AP,panel_one,3\nAP,panel_two,2\n");
	build_string += "AD,1,0,test_2,12\nAD,1,0,test_3,8\nAD,1,0,test_4,12\n";
	build_string += "AS,0,0,switch_2,false,false\nAS,0,0,switch_3,false,false\n";
	build_string += "AB,0,2,switch_bank_0,12,false,false\nAB,1,1,switch_bank_1,16,true,false\n";
	pm.Build(build_string);
	
	pm.AddDisplay(0, 0, "test_0", 12);
	pm.AddDisplay(0, 0, "test_1", 18);
	
	pm.AddSwitch(1, 1, "switch_012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789", false, false);
	pm.AddSwitch(1, 1, "switch_1", true, false);
	
	bool finished = false;
	while (!finished) {
		for (int i = 0; i < 256; i ++) {
			pm.SetDisplayValue(0, 0, i, false);
			pm.SetDisplayValue(0, 1, 2 * i, false);
			pm.SetDisplayValue(1, 0, i, false);
			pm.SetDisplayValue(1, 1, 2 * i, false);
			if (pm.CheckInput(false)) {
				finished = true;
				break;
			}
			wrefresh(mainwin);
			usleep(100000);
		}
	}
	
	endwin();			/* End curses mode		  */

	return 0;
}
