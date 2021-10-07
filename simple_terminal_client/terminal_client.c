#include <curses.h>			// ncurses.
#include <fcntl.h>			// open() and O_RDONLY, O_WRONLY, etc.
#include <unistd.h>			// write(), read(), sleep().
#include <string.h>			// strcmp().
#include <stdlib.h>			// exit(), EXIT_FAILURE.

void CleanUp(WINDOW* win);
void PrintCharacter(char character, int *cursor_y_pos, int *cursor_x_pos, int max_y, int max_x);
int  InsertNewline(int *cursor_y_pos, int *cursor_x_pos, int max_y);

int main(int argc, char *argv[]) {
    // FIFO bits.
	int bytes_in;
	int fifo_data_slave_to_master;
	int fifo_data_master_to_slave;
	int fifo_cmd_master_to_slave;
	char* fifo_data_slave_to_master_path = argv[1];
	char* fifo_data_master_to_slave_path = argv[2];
	char* fifo_cmd_master_to_slave_path = argv[3];
	fifo_data_slave_to_master = open(fifo_data_slave_to_master_path, O_WRONLY);
	fifo_data_master_to_slave = open(fifo_data_master_to_slave_path, O_RDONLY | O_NONBLOCK);
	fifo_cmd_master_to_slave = open(fifo_cmd_master_to_slave_path, O_RDONLY | O_NONBLOCK);
	printf("Checkpoint.\n");
    /*  Initialize ncurses  */
    WINDOW* mainwin;
    if ( (mainwin = initscr()) == NULL ) {
	fprintf(stderr, "Error initializing ncurses.\n");
	exit(EXIT_FAILURE);
    }
    noecho();					/*  Turn off key echoing                 */
    keypad(mainwin, TRUE);		/*  Enable the keypad for non-char keys  */
    scrollok(mainwin, TRUE);	/*  Enable window scrolling when we call scroll()  */
    // Get terminal emulator window dimensions.
    int max_x;
    int max_y;
    getmaxyx(mainwin, max_y, max_x);
    timeout(5);
    
    int cursor_x_pos = 0;
    int cursor_y_pos = 0;
    move(cursor_x_pos, cursor_y_pos);
    refresh();
    
	char end_code[] = "end";
	char match_buffer[20];
	int match_index = 0;
	char command_buffer[20];
	int command_index = 0;
	
	while (true) {
		int in_key = getch();
		if (in_key != ERR) {
			write(fifo_data_slave_to_master, &in_key, 1);
			switch (in_key) {
				case 10:
					if (InsertNewline(&cursor_y_pos, &cursor_x_pos, max_y) > 0) {
						scroll(mainwin);
					}
					break;
				default:
					PrintCharacter(in_key, &cursor_y_pos, &cursor_x_pos, max_y, max_x);
					break;
			}
		}
		char data_read_in;
		int ret_val = read(fifo_data_master_to_slave, &data_read_in, 1);
		if (((ret_val == 0) || (ret_val == -1)) == false) {
			switch (data_read_in) {
				case '\n':
					if (InsertNewline(&cursor_y_pos, &cursor_x_pos, max_y) > 0) {
						scroll(mainwin);
					}
					break;
				default:
					PrintCharacter(data_read_in, &cursor_y_pos, &cursor_x_pos, max_y, max_x);
					break;
			}
			
		}
		char cmd_in;
		while (read(fifo_cmd_master_to_slave, &cmd_in, 1) > 0) {
			if (cmd_in != '\n') {
				command_buffer[command_index] = cmd_in;
				if (command_index < 20) {
					command_index += 1;
				}
			} else {
				command_buffer[command_index] = '\0';
				command_index = 0;
				if (strcmp(end_code, command_buffer) == 0) {
					CleanUp(mainwin);
					return 0;
				}
			}
		}
		refresh();
	}
	CleanUp(mainwin);
    return 0;
}

void CleanUp(WINDOW* win) {
	/*  Clean up after ourselves  */
	delwin(win);
	endwin();
	refresh();
}

void PrintCharacter(char character, int *cursor_y_pos, int *cursor_x_pos, int max_y, int max_x) {
	addch(character);
	if (*cursor_y_pos < (max_y - 1)) {
		// If we are not on the last row, if we are on the last column the preceeding addch() call
		// will *not* change the cursor position, so we do it manually.
		if (*cursor_x_pos == (max_x - 1)) {
			*cursor_x_pos = 0;
			*cursor_y_pos += 1;
		} else {
			*cursor_x_pos += 1;
		}
	} else if (*cursor_y_pos == (max_y - 1)) {
		// However, if we are on the last row, if we are on the last column when addch() is called
		// the the window is scrolled automatically, so all we need to do is set the cursor x = 0.
		if (*cursor_x_pos == (max_x - 1)) {
			*cursor_x_pos = 0;
		} else {
			*cursor_x_pos += 1;
		}
	}
	move(*cursor_y_pos, *cursor_x_pos);
}

int InsertNewline(int *cursor_y_pos, int *cursor_x_pos, int max_y) {
	*cursor_x_pos = 0;
	*cursor_y_pos += 1;
	int scroll = 0;
	if (*cursor_y_pos > (max_y - 1)) {
		// Scrolling seems to happen automatically if you roll-over from the end of the last row,
		// but if we are going past the last row by hitting return, we need to force it with scroll().
		*cursor_y_pos = (max_y - 1);
		scroll = 1;
	}
	move(*cursor_y_pos, *cursor_x_pos);
	return scroll;
}
