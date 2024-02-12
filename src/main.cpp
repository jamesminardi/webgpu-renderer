#include <iostream>

#include "stb_image_write.h"
#include "application.h"

#include "terrain.h"
#include "noise/noise.h"


int main (int, char**) {

	Noise noise(0);

	noise.generate(1.0f, {0, 0});

	noise.output();
//	noise.outputColor();

// Terrain App
//	try {
//		auto app = Application();
//
//		while (app.isRunning()) {
//			app.onFrame();
//		}
//	}
//	catch (const std::runtime_error& e) {
//		std::cerr << "[Runtime Exception] " << e.what() << '\n';
//	}
//	catch(const std::exception& e) {
//		std::cerr << e.what() << '\n';
//	}

	return 0;
}