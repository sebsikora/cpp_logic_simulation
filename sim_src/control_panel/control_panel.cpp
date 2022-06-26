#include <string>

#include <ncurses.h>
#include <fcntl.h>			// open() and O_RDONLY, O_WRONLY, etc.
#include <unistd.h>			// write(), read(), sleep().
#include <string.h>			// strcmp().
#include <stdlib.h>			// exit(), EXIT_FAILURE.

#include "panel_mgr.h"

int main(int argc, char *argv[]) {
    // FIFO bits.
	int bytes_in;
	int fifo_data_slave_to_master;
	int fifo_data_master_to_slave;
	char* fifo_data_slave_to_master_path = argv[1];
	char* fifo_data_master_to_slave_path = argv[2];
	fifo_data_slave_to_master = open(fifo_data_slave_to_master_path, O_WRONLY);
	fifo_data_master_to_slave = open(fifo_data_master_to_slave_path, O_RDONLY | O_NONBLOCK);
	printf("Checkpoint.\n");
	sleep(5);
	
	WINDOW* mw = initscr();			/* Start curses mode 		  */
	int max_y;
    int max_x;
    getmaxyx(mw, max_y, max_x);

	WINDOW* mainwin;
	mainwin = newwin(max_y, max_x, 0, 0);
	PanelManager pm(mainwin, "test_panel");
	
	std::string command_buffer = "";
	
	bool finished = false;
	while (!finished) {
		if (pm.CheckInput(false)) {
			finished = true;
		}
		
		char cmd_in;
		while (read(fifo_data_master_to_slave, &cmd_in, 1) > 0) {
			if (cmd_in != '\n') {
				command_buffer += std::string(1, cmd_in);
			} else {
				if (command_buffer == "q") {
					finished = true;
				} else {
					pm.Build(command_buffer + "\n");
				}
				command_buffer = "";
			}
		}
		
		wrefresh(mainwin);
	}
	
	delwin(mainwin);
	endwin();				/* End curses mode		  */
	refresh();
	
	return 0;
}
