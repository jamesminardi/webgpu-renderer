#include "application.h"

int main (int, char**) {

	try {
		auto app = Application();

		while (app.isRunning()) {
			app.onFrame();
		}
	}
	catch (const std::runtime_error& e) {
		std::cerr << "[Runtime Exception] " << e.what() << '\n';
	}
	catch(const std::exception& e) {
		std::cerr << e.what() << '\n';
	}

	return 0;
}