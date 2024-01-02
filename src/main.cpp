#include "application.h"

int main (int, char**) {

	auto app = Application();

	while (app.isRunning()) {
		app.onFrame();
	}

	return 0;
}