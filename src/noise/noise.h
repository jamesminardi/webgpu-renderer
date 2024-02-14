#pragma once

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
// Include glm constants
#include <glm/gtc/constants.hpp>
#include <stb_image_write.h>
#include <cmath>


class Hash {
public:

	// Hashing
	static const int PrimeX = 501125321;
	static const int PrimeY = 1136930381;
	static const int PrimeZ = 1720413743;

	int hash1(int seed, int xPrimed, int yPrimed) {

		int hash = seed ^ xPrimed ^ yPrimed;

		hash *= 0x27d4eb2d;
		return hash;
	}

	int hash2(int seed, int xPrimed, int yPrimed) {
		int a = 1103515245;
		int b = 12345;
		int n = xPrimed + yPrimed * 57;
		return (a * (n + seed) + b) & 0x7fffffff;
	}

	float hash3(int seed, int xPrimed, int yPrimed) {
		int hash = seed ^ xPrimed ^ yPrimed;
		hash *= 0x27d4eb2d;
		return glm::fract(hash * 43758.5453f);
	}

	float valCoord(int seed, int xPrimed, int yPrimed)
	{
		int hash = hash1(seed, xPrimed, yPrimed);

		hash *= hash;
		hash ^= hash << 19;
		return hash * (1 / 2147483648.0f);
	}

	// Evaluate cubic interpolation at a given point.
	float evalBicubic2(float x, float y) {

		float xFrac = glm::fract(x);
		float yFrac = glm::fract(y);

		// Generate the primed coordinates for the 16 surrounding points.
		int x1 = glm::floor(x) * PrimeX;
		int y1 = glm::floor(y) * PrimeY;
		int x0 = x1 - PrimeX;
		int y0 = y1 - PrimeY;
		int x2 = x1 + PrimeX;
		int y2 = y1 + PrimeY;
		int x3 = x1 + (int)((long)PrimeX << 1); // Advance two steps
		int y3 = y1 + (int)((long)PrimeY << 1);

		// First Row
		float c00 = valCoord(0, x0, y0);
		float c10 = valCoord(0, x1, y0);
		float c20 = valCoord(0, x2, y0);
		float c30 = valCoord(0, x3, y0);

		// Second Row
		float c01 = valCoord(0, x0, y1);
		float c11 = valCoord(0, x1, y1);
		float c21 = valCoord(0, x2, y1);
		float c31 = valCoord(0, x3, y1);

		// Third Row
		float c02 = valCoord(0, x0, y2);
		float c12 = valCoord(0, x1, y2);
		float c22 = valCoord(0, x2, y2);
		float c32 = valCoord(0, x3, y2);

		// Fourth Row
		float c03 = valCoord(0, x0, y3);
		float c13 = valCoord(0, x1, y3);
		float c23 = valCoord(0, x2, y3);
		float c33 = valCoord(0, x3, y3);

		float r0 = cubicInterpolate(c00, c10, c20, c30, xFrac);
		float r1 = cubicInterpolate(c01, c11, c21, c31, xFrac);
		float r2 = cubicInterpolate(c02, c12, c22, c32, xFrac);
		float r3 = cubicInterpolate(c03, c13, c23, c33, xFrac);


		// Todo: Do we need to clamp? What is the range of the noise?
		// Todo: How does the 1.5f factor affect the noise?
		return cubicInterpolate(r0, r1, r2, r3, yFrac) * (1 / (1.5f * 1.5f));

	}



class Noise {
public:

	enum class Interpolation {
		Linear,
		Cosine,
		Smoothstep,
		Smootherstep,
		Cubic
	};

	float hash(glm::vec2 uv) {
		return glm::fract(glm::sin(glm::dot(uv, glm::vec2(12.9898f, 78.233f))) * 43758.5453f);
	}

	Noise (int seed = 0) {
		std::srand(seed);
		for (int i = 0; i < tableSize; i++) {
			for (int j = 0; j < tableSize; j++) {
				table[i][j] = hash(glm::vec2(i, j));
			}
		}
	}


	glm::vec2 cosineStep(const glm::vec2 &f) {
		return (glm::vec2(1.0f, 1.0f) - glm::cos(f * glm::pi<float>())) * 0.5f;
	}
	glm::vec2 smoothStep(const glm::vec2 &f) {
			return f * f * (3.0f - 2.0f * f);
	}
	glm::vec2 smootherStep(const glm::vec2 &f) {
		return f * f * f * (f * (f * 6.0f - 15.0f) + 10.0f);
	}

	float cubicInterpolate(float a, float b, float c, float d, float f) {
		// n=x(x(x(−a+b−c+d)+2a−2b+c−d)−a+c)+b
		return f * (f * (f * (-a + b - c + d) + (2.0f * a) - 2.0f * b + c - d) - a + c) + b;
	}

	// Evaluate cubic interpolation at a given point.
	float evalBicubic(glm::vec2 uv) {

		glm::ivec2 min = glm::floor(uv);

		// Interpolate
		glm::vec2 f = glm::fract(uv);
		float xSamples[4];
		glm::vec4 samples;

		for(int i = 0; i < 4; i++) {
			int y = ((min.y - 1 + i) % tableSize + tableSize) % tableSize;
			int x1 = ((min.x - 1) % tableSize + tableSize) % tableSize;
			int x2 = ((min.x + 0) % tableSize + tableSize) % tableSize;
			int x3 = ((min.x + 1) % tableSize + tableSize) % tableSize;
			int x4 = ((min.x + 2) % tableSize + tableSize) % tableSize;

			// Interpolate across each sample in the row
			xSamples[i] = cubicInterpolate(
					table[y][x1],
					table[y][x2],
					table[y][x3],
					table[y][x4],
					f.x);
		}

		// Interpolate across each row
		return glm::clamp(cubicInterpolate(xSamples[0], xSamples[1], xSamples[2], xSamples[3], f.y), 0.0f, 1.0f);

	}

	/**
	 * @brief Evaluate the noise at a given point using bilinear interpolation
	 * @param uv The 2D point at which to evaluate the noise.
	 * @param interpolationMethod The interpolation method to use.
	 * @return The evaluated noise value.
	 */
	float evalBilinear(glm::vec2 uv, Interpolation interpolationMethod = Interpolation::Linear) {

		glm::ivec2 min = glm::floor(uv);
		min = (min % tableSize + tableSize) % tableSize;

		glm::ivec2 max = min + 1;
		max = (max % tableSize + tableSize) % tableSize;

		float tl = table[min.y][min.x];
		float bl = table[max.y][min.x];
		float tr = table[min.y][max.x];
		float br = table[max.y][max.x];

		// Interpolate
		glm::vec2 f = glm::fract(uv);

		switch (interpolationMethod) {
			case Interpolation::Linear:
				break;
			case Interpolation::Cosine:
				f = cosineStep(f);
				break;
			case Interpolation::Smoothstep:
				f = smoothStep(f);
				break;
			case Interpolation::Smootherstep:
				f = smootherStep(f);
				break;
			default:
				break;
		}

		// Bilerp between the four corners
		float top = glm::mix(tl, tr, f.x);
		float bot = glm::mix(bl, br, f.x);
		return glm::mix(top, bot, f.y);
	}


	float fbm(glm::vec2 uv, int numOctaves = 3, float lacunarity = 2.0f) {

//		return evalBilinear(uv, Interpolation::Smoothstep);

		constexpr float gain = 0.5f; // Gain of 0.5 is standard form fBm
		float frequency = 1.0f;
		float amplitude = 0.5f;
		float t = 0.0f;

		for (int i = 0; i < numOctaves; i++) {
			t += amplitude * evalBicubic(frequency * uv);
			frequency *= lacunarity; // How much detail is added or removed at each octave (Adjusts frequency)
			amplitude *= gain;
		}
		return glm::clamp(t, 0.0f, 1.0f);
	}


	/**
	 * @brief Generates a noise map with a given offset.
	 * @param offset The offset to apply to the noise map.
	 * @param period The period of hte noise map.
	 *
	 * @details The noise map is generated by evaluating the noise
	 * 			at each point in the map using bilinear interpolation.
	 */
	void generate(float period = 1.0f, glm::vec2 offset = glm::vec2(0.0f)) {

		int stepsPerUnit = mapSize / tableSize;

		for (int row = 0; row < mapSize; row++) {
			for (int col = 0; col < mapSize; col++) {
				float x = (col / float(stepsPerUnit) + offset.x) / period;
				float y = (row / float(stepsPerUnit) + offset.y) / period;
				bitMap[row][col] = evalBicubic(glm::vec2(x, y)) * 255;
			}
		}
	}




	void output() {

//		for (int row = 0; row < mapSize; row++) {
//			for (int col = 0; col < mapSize; col++) {
//				bitMap[row][col] = noiseMap[row][col] * 255;
//			}
//		}



		if(stbi_write_bmp("noise.bmp", Noise::mapSize, Noise::mapSize, 1, bitMap) == 0) {
			std::cerr << "Failed to write noise to file" << std::endl;
		} else {
			std::cout << "Wrote noise to file" << std::endl;
		}

	}




//	void outputColor() {
//		//rgb
//		uint8_t water[] = {0, 40, 240};
//		uint8_t sand[] = {255, 240, 15};
//		uint8_t grass[] = {20, 200, 20};
//		uint8_t rock[] = {128, 128, 128};
//		uint8_t snow[] = {240, 245, 255};
//
//
//
//		for (int row = 0; row < mapSize; row++) {
//			for (int col = 0; col < mapSize; col++) {
//				if (bitMap[row][col] < 100) {
//					colorMap[row][col][0] = water[0];
//					colorMap[row][col][1] = water[1];
//					colorMap[row][col][2] = water[2];
//				}
//				else if (bitMap[row][col] < 110) {
//					colorMap[row][col][0] = sand[0];
//					colorMap[row][col][1] = sand[1];
//					colorMap[row][col][2] = sand[2];
//				}
//				else if (bitMap[row][col] < 160) {
//					colorMap[row][col][0] = grass[0];
//					colorMap[row][col][1] = grass[1];
//					colorMap[row][col][2] = grass[2];
//				}
//				else if (bitMap[row][col] < 190) {
//					colorMap[row][col][0] = rock[0];
//					colorMap[row][col][1] = rock[1];
//					colorMap[row][col][2] = rock[2];
//				}
//				else {
//					colorMap[row][col][0] = snow[0];
//					colorMap[row][col][1] = snow[1];
//					colorMap[row][col][2] = snow[2];
//				}
//			}
//		}
//
//
//		if(stbi_write_bmp("noise_color.bmp", Noise::mapSize, Noise::mapSize, 3, colorMap) == 0) {
//			std::cerr << "Failed to write noise to file" << std::endl;
//		} else {
//			std::cout << "Wrote noise to file" << std::endl;
//		}
//
//	}

	// Size of the table used to generate the noise.
	static const int tableSize = 16;

	// Width and height of the noise map.
	// I.e. If resolution is 64 and table size is 16, there will be
	// four steps per random value in the table.
	static const int mapSize = 128;

	// The generated noise map. The entire noise map interpolates across
	// the table of random values.
//	float noiseMap[mapSize][mapSize]{};

	// The bitmap representation of the noise map used for output files.
	uint8_t bitMap[mapSize][mapSize]{};

//	uint8_t colorMap[mapSize][mapSize][3]{};

	// Table of random values used to generate the noise.
	float table[tableSize][tableSize]{};

};