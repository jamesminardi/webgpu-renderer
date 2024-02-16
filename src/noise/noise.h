#pragma once

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
// Include glm constants
#include <glm/gtc/constants.hpp>
#include <stb_image_write.h>
#include <cmath>
#include <vector>


class Noise {
public:

	enum Function {
		Value,
		ValueCubic,
		Simplex,
		Perlin,
		Cellular
	};

	enum class Interpolation {
		Linear,
		Cosine,
		Smoothstep,
		Smootherstep
	};

	enum class Fractal {
		None,
		FBM,
		Ridged,
		Turbulence,
		DomainWarp,
	};

	static constexpr float Gain = 0.5f; // Gain of 0.5 is standard form fBm
	Function noiseFunction;
	Interpolation interpolationMethod;
	Fractal fractalMethod;
	int fractalOctaves;

	int m_seed;
	float m_lacunarity;
	float m_weightedStrength;

	Noise(int seed = 0) : m_seed(seed), m_lacunarity(2.0f), m_weightedStrength(0.0f) {

		noiseFunction = Function::Value;
		interpolationMethod = Interpolation::Linear;
		fractalMethod = Fractal::None;
		fractalOctaves = 1;

	}


	// Hashing
	// ----------------------------

	// Magic hash constants
	static const int PrimeX = 501125321;
	static const int PrimeY = 1136930381;
	static const int PrimeZ = 1720413743;
	static const int PrimeW = 0x27d4eb2d; // 668265261

	static int hashInt(int seed, int xPrimed, int yPrimed) {

		// Combine seed and primes using XOR.
		// XOR is reversible, commutative, associative, and mixes bit values well.
		int hash = seed ^ xPrimed ^ yPrimed;
		hash *= PrimeW;
		return hash;
	}


	// Get a hashed float in range [0, 1]
	static float hashFloat(int seed, int xPrimed, int yPrimed)
	{
		int h = hashInt(seed, xPrimed, yPrimed);
		// Make positive and scale to [0, 1]
		h &= 0x7fffffff; // Ensure hash is positive integer
		return h / 2147483647.0f; // Max 31bit unsigned integer

	}



	// Interpolation
	// ----------------------------

	// Interpolate between two values using weight t
	static float lerp(const float &a, const float &b, const float &t) {
		return a + t * (b - a);
	}

	// Interpolate between four values using weight t
	static float cubicLerp(const float &a, const float &b, const float &c, const float &d, const float &t) {
		// Old:
		// return f * (f * (f * (-a + b - c + d) + (2.0f * a) - 2.0f * b + c - d) - a + c) + b;

		// Updated:
		float p = (d - c) - (a - b);
		float q = (a - b) - p;
		float r = c - a;
		float s = b;
		return (p * t * t * t) + (q * t * t) + (r * t) + s;
	}

	// Hermite/Smoothstep Interpolation. t : 0..1
	static glm::vec2 smoothStep(const glm::vec2 &t) {
		return t * t * (3.0f - 2.0f * t);
	}

	// Quintic Interpolation
	static glm::vec2 smootherStep(const glm::vec2 &t) {
		return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
	}

	// Given an integer point on a grid, hash the point.
	static float evalGrid(int seed, glm::ivec2 p) {
		return evalGrid(seed, p.x, p.y);
	}

	static float evalGrid(int seed, int x, int y) {
		return hashFloat(seed, (x ^ PrimeX) * PrimeX, (y ^ PrimeY) * PrimeY);
	}


	// Given a float point, evaluate the value noise using the surrounding 3x3 grid of integer points.
	static float eval(int seed, glm::vec2 p, Interpolation interpolation = Interpolation::Smoothstep) {

		int x0 = (int(glm::floor(p.x)) ^ PrimeX) * PrimeX;
		int y0 = (int(glm::floor(p.y)) ^ PrimeY) * PrimeY;

		int x1 = x0 + PrimeX;
		int y1 = y0 + PrimeY;

		float c00 = hashFloat(seed, x0, y0);
		float c10 = hashFloat(seed, x1, y0);
		float c01 = hashFloat(seed, x0, y1);
		float c11 = hashFloat(seed, x1, y1);

		glm::vec2 t = glm::fract(p);

		switch (interpolation) {
			case Interpolation::Linear:
				break;
			case Interpolation::Smoothstep:
				t = smoothStep(t);
				break;
			case Interpolation::Smootherstep:
				t = smootherStep(t);
				break;
			default:
				break;
		}
		// Linear interpolate between the four corners
		float top = lerp(c00, c10, t.x);
		float bot = lerp(c01, c11, t.x);
		return lerp(top, bot, t.y);
	}


	// Given a float point, evaluate the value noise using the surrounding 3x3 grid of integer points.
	float evalCubic(int seed, glm::vec2 p) {

		float xFrac = glm::fract(p.x);
		float yFrac = glm::fract(p.y);

		// Generate the primed coordinates for the 16 surrounding points.
		int x1 = (int(glm::floor(p.x)) ^ PrimeX) * PrimeX;
		int y1 = (int(glm::floor(p.y)) ^ PrimeY) * PrimeY;
		int x0 = x1 - PrimeX;
		int y0 = y1 - PrimeY;
		int x2 = x1 + PrimeX;
		int y2 = y1 + PrimeY;
		int x3 = x1 + (int)((long)PrimeX << 1); // Advance two steps
		int y3 = y1 + (int)((long)PrimeY << 1);

		// First Row
		float c00 = hashFloat(seed, x0, y0);
		float c10 = hashFloat(seed, x1, y0);
		float c20 = hashFloat(seed, x2, y0);
		float c30 = hashFloat(seed, x3, y0);

		// Second Row
		float c01 = hashFloat(seed, x0, y1);
		float c11 = hashFloat(seed, x1, y1);
		float c21 = hashFloat(seed, x2, y1);
		float c31 = hashFloat(seed, x3, y1);

		// Third Row
		float c02 = hashFloat(seed, x0, y2);
		float c12 = hashFloat(seed, x1, y2);
		float c22 = hashFloat(seed, x2, y2);
		float c32 = hashFloat(seed, x3, y2);

		// Fourth Row
		float c03 = hashFloat(seed, x0, y3);
		float c13 = hashFloat(seed, x1, y3);
		float c23 = hashFloat(seed, x2, y3);
		float c33 = hashFloat(seed, x3, y3);

		// Interpolate each row
		float r0 = cubicLerp(c00, c10, c20, c30, xFrac);
		float r1 = cubicLerp(c01, c11, c21, c31, xFrac);
		float r2 = cubicLerp(c02, c12, c22, c32, xFrac);
		float r3 = cubicLerp(c03, c13, c23, c33, xFrac);


		// Todo: Do we need to clamp? What is the range of the noise?
		// Todo: How does the 1.5f factor affect the noise?
		// Interpolate the rows
		return cubicLerp(r0, r1, r2, r3, yFrac) * (1 / (1.5f * 1.5f));

	}

	// Todo: Check correctness compared to old version
	float evalFBm(glm::vec2 p) {

		int s = m_seed;
		float amp = 0.5f;
		float freq = 1.0f;
		float sum = 0.0f;

		for (int i = 0; i < fractalOctaves; i++) {
			float noise = eval(s, p * freq, interpolationMethod);
			s++;
			sum += amp * noise;

			amp *= lerp(1.0f, (noise + 1) * 0.5f, m_weightedStrength);
			amp *= Gain;

			freq *= m_lacunarity; // How much detail is added or removed at each octave (Adjusts frequency)
		}

		return sum;

	}
};


class NoiseTable {
public:
	constexpr static int DefaultSize = 16;
	constexpr static int DefaultSeed = 0;
	Noise noise;
	std::vector<uint8_t> table;
	int size = DefaultSize;

	NoiseTable(int seed = DefaultSeed) {
		noise = Noise(seed);
//		table.reserve(DefaultSize * DefaultSize);
	}

	void generate(int tableSize = DefaultSize) {
		size = tableSize;
		table.resize(size * size);
		for (int y = 0; y < size; y++) {
			for (int x = 0; x < size; x++) {
				table[y * size + x] = static_cast<uint8_t>(Noise::evalGrid(noise.m_seed, x, y) * 255);
			}
		}
	}

	void output() {

		// Todo: use unused filename
		if(stbi_write_bmp("noise.bmp", size, size, 1, table.data()) == 0) {
			std::cerr << "Failed to write noise to file" << std::endl;
		} else {
			std::cout << "Wrote noise to file" << std::endl;
		}

	}

};


///**
// * @brief Generates a noise map with a given offset.
// * @param offset The offset to apply to the noise map.
// * @param period The period of hte noise map.
// *
// * @details The noise map is generated by evaluating the noise
// * 			at each point in the map using bilinear interpolation.
// */
//void generate(float period = 1.0f, glm::vec2 offset = glm::vec2(0.0f)) {
//
//	int stepsPerUnit = mapSize / tableSize;
//
//	for (int row = 0; row < mapSize; row++) {
//		for (int col = 0; col < mapSize; col++) {
//			float x = (col / float(stepsPerUnit) + offset.x) / period;
//			float y = (row / float(stepsPerUnit) + offset.y) / period;
//			bitMap[row][col] = evalBicubic(glm::vec2(x, y)) * 255;
//		}
//	}
//}
