#include <iostream>

#include "stb_image_write.h"
#include "application.h"

#include "terrain.h"
#include "noise/noise.h"


int main (int, char**) {

	// int stbi_write_bmp(char const *filename, int w, int h, int comp, const void *data);

	Noise noise(0);

	noise.generate(1.0f, {0, 0});

	noise.output();
//	noise.outputColor();

//	noise.outputRandomNoise();

//	ValueNoise1D valueNoise1D(0);
//
//	static const int numSteps = 50;
//
//	for (int i = 0; i < numSteps; ++i) {
//		float x = i / float(numSteps - 1) * 10;
//		std::cout << "Noise at " << x << ": " << valueNoise1D.linearEval2(x) << std::endl;
//	}
//	std::cout << std::endl;
//
//
//	for (int i = 0; i < numSteps; ++i) {
//		float x = i / float(numSteps - 1) * 10;
//		std::cout << "Noise at " << x << ": " << valueNoise1D.smoothEval2(x) << std::endl;
//	}

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