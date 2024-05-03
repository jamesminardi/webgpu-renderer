#include <iostream>

#include "stb_image_write.h"
#include "globals.h"
#include "application.h"


int main (int, char**) {

//	Noise noise(0);
//
//	noise.generate(1.0f, {0, 0});
//
//	noise.output();
//	noise.outputColor();

//	NoiseTable noiseTable;

//	Noise noise;

//	noise.noiseFunction = Noise::Function::Value;
//	noise.interpolationMethod = Noise::Interpolation::Linear;
//	noise.fractalMethod = Noise::Fractal::FBM;
//	noise.fractalOctaves = 3;
//	noiseTable.generate(noise, 128, 16);
//	noiseTable.output("noise_16to128_value_linear_fbm_03.bmp");
//
//
//	noise.noiseFunction = Noise::Function::Value;
//	noise.interpolationMethod = Noise::Interpolation::Smoothstep;
//	noise.fractalMethod = Noise::Fractal::FBM;
//	noise.fractalOctaves = 3;
//	noiseTable.generate(noise, 128, 16);
//	noiseTable.output("noise_16to128_value_smooth_fbm_03.bmp");
//
//
//	noise.noiseFunction = Noise::Function::Value;
//	noise.interpolationMethod = Noise::Interpolation::Smootherstep;
//	noise.fractalMethod = Noise::Fractal::FBM;
//	noise.fractalOctaves = 3;
//	noiseTable.generate(noise, 128, 16);
//	noiseTable.output("noise_16to128_value_smoother_fbm_03.bmp");

//	noise.noiseFunction = Noise::Function::ValueCubic;
//	noise.interpolationMethod = Noise::Interpolation::Linear;
//	noise.fractalMethod = Noise::Fractal::None;
//	noise.fractalOctaves = 1;
//	noiseTable.generate(noise, 128, 16);
//	noiseTable.output("ws_none_w.bmp");
//
//	noise.noiseFunction = Noise::Function::ValueCubic;
//	noise.interpolationMethod = Noise::Interpolation::Linear;
//	noise.fractalMethod = Noise::Fractal::FBM;
//	noise.fractalOctaves = 1;
//	noiseTable.generate(noise, 128, 16);
//	noiseTable.output("ws_o1_w.bmp");
//
//	noise.noiseFunction = Noise::Function::ValueCubic;
//	noise.interpolationMethod = Noise::Interpolation::Linear;
//	noise.fractalMethod = Noise::Fractal::FBM;
//	noise.fractalOctaves = 2;
//	noiseTable.generate(noise, 128, 16);
//	noiseTable.output("ws_o2_w.bmp");
//
//	noise.noiseFunction = Noise::Function::ValueCubic;
//	noise.interpolationMethod = Noise::Interpolation::Linear;
//	noise.fractalMethod = Noise::Fractal::FBM;
//	noise.fractalOctaves = 3;
//	noiseTable.generate(noise, 128, 16);
//	noiseTable.output("ws_o3_w.bmp");



	// Terrain App

	Application* app;

	try {
        app = new Application();

		while (app->isRunning()) {
            app->onFrame();
		}
	}
	catch (const std::runtime_error& e) {
		std::cerr << "[Runtime Exception] " << e.what() << '\n';
	}
	catch(const std::exception& e) {
		std::cerr << e.what() << '\n';
	}

	delete app;

	return 0;
}