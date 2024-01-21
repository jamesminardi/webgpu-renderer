#pragma once

#include <cmath>

struct HSV {
	float h = 0.0f; // [0.0, 360.0] Hue in Degrees
	float s = 1.0f; // [0.0, 1.0]	saturation
	float v = 1.0f; // [0.0, 1.0]	value
};

struct RGB {
	float r = 1.0f; // [0.0, 1.0]
	float g = 1.0f; // [0.0, 1.0]
	float b = 1.0f; // [0.0, 1.0]
};

RGB hsv2RGB(const HSV hsv) {
	RGB rgb;
	// Normalize hue to [0.0, 360.0]
	float n = std::fmod((std::fmod(hsv.h, 360.0f) + 360.0f), 360.0f);
	float c = hsv.v * hsv.s;
	float x = c * (1.0f - std::abs(std::fmod(n / 60.0f, 2.0f) - 1.0f));
	float m = hsv.v - c;

	if (n >= 0.0f && n < 60.0f) {
		rgb.r = c;
		rgb.g = x;
		rgb.b = 0.0f;
	}
	else if (n >= 60.0f && n < 120.0f) {
		rgb.r = x;
		rgb.g = c;
		rgb.b = 0.0f;
	}
	else if (n >= 120.0f && n < 180.0f) {
		rgb.r = 0.0f;
		rgb.g = c;
		rgb.b = x;
	}
	else if (n >= 180.0f && n < 240.0f) {
		rgb.r = 0.0f;
		rgb.g = x;
		rgb.b = c;
	}
	else if (n >= 240.0f && n < 300.0f) {
		rgb.r = x;
		rgb.g = 0.0f;
		rgb.b = c;
	}
	else if (n >= 300.0f && n < 360.0f) {
		rgb.r = c;
		rgb.g = 0.0f;
		rgb.b = x;
	}

	rgb.r += m;
	rgb.g += m;
	rgb.b += m;

	return rgb;
}
