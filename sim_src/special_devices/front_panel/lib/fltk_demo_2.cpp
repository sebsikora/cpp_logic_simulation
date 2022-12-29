#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Toggle_Button.H>

#include "toggle_switches.h"

#include <iostream>

static void defaultClose_callback(Fl_Widget* widget, void* v) {}

int main(int argc, char **argv) {
	Fl_Window *window = new Fl_Window(300,180);

	//~window->callback(defaultClose_callback);

	PanelManager* pm = new PanelManager("test",
										0,
										0,
										300,
										180,
										{"toggle, toggle, 6, 0, 0, 70, 30, 30",
										 "momentary, momentary, 6, 0, 40, 70, 30, 30",
										 "radio, radio, 6, A. B. C. D. E. F, 0, 80, 70, 30, 30",
										 "indicator, lamps, 6, 0, 120, 70, 30, 30"
										});
	
	window->end();
	window->show(argc, argv);
	return Fl::run();
}

