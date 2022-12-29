#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Toggle_Button.H>

#include "toggle_switches.h"

#include <iostream>

static void defaultClose_callback(Fl_Widget* widget, void* v) {}

int main(int argc, char **argv) {
	Fl_Window *window = new Fl_Window(300,180);

	//~window->callback(defaultClose_callback);

	PanelManager* pm = new PanelManager("test", 0, 0, 300, 180);
	
	window->end();
	window->show(argc, argv);
	return Fl::run();
}

