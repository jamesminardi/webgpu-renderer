#pragma once

#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
// Include glm constants
#include <glm/gtc/constants.hpp>
#include <stb_image_write.h>

class Noise {
public:

	enum class Interpolation {
		Linear,
		Cosine,
		Smoothstep,
		Smootherstep
	};

	float hash(glm::vec2 uv) {
		return glm::fract(glm::sin(glm::dot(uv, glm::vec2(12.9898f, 78.233f))) * 43758.5453f);
	}

	Noise (int seed = 0) {
		std::srand(0);
		for (int i = 0; i < tableSize; i++) {
			for (int j = 0; j < tableSize; j++) {
				table[i][j] = std::rand() / float(RAND_MAX);
			}
		}
	}

	float evalSmoothBilerp(glm::vec2 uv, Interpolation smooth = Interpolation::Linear) {
		int xMin = glm::floor(uv.x);
		int yMin = glm::floor(uv.y);

		// Wrap around on edges
		int xMax = (xMin + 1) % tableSize;
		int yMax = (yMin + 1) % tableSize;

		float tl = table[xMin][yMin];
		float bl = table[xMax][yMin];
		float tr = table[xMin][yMax];
		float br = table[xMax][yMax];

		// Interpolate
		glm::vec2 f = glm::fract(uv);

		switch (smooth) {
			case Interpolation::Linear:
				break;
			case Interpolation::Cosine:
				f = (glm::vec2(1.0f, 1.0f) - glm::cos(f * glm::pi<float>())) * 0.5f;
				break;
			case Interpolation::Smoothstep:
				f = f * f * (3.0f - 2.0f * f);
				break;
			case Interpolation::Smootherstep:
				f = f * f * f * (f * (f * 6.0f - 15.0f) + 10.0f);
				break;
			default:
				break;
		}

		// Bilerp between the four corners
		float left = glm::mix(tl, bl, f.x);
		float right = glm::mix(tr, br, f.x);
		return glm::mix(left, right, f.y);
	}

	void generate() {

		int stepsPerUnit = resolution / tableSize;

		for (int row = 0; row < resolution; row++) {
			for (int col = 0; col < resolution; col++) {
				float x = col / float(stepsPerUnit);
				float y = row / float(stepsPerUnit);
				noiseMap[row][col] = evalSmoothBilerp(glm::vec2(x, y), Interpolation::Smootherstep);
			}
		}
	}

	void output() {

		for (int row = 0; row < resolution; row++) {
			for (int col = 0; col < resolution; col++) {
				bitMap[row][col] = noiseMap[row][col] * 255;
			}
		}

		if(stbi_write_bmp("noise.bmp", Noise::resolution, Noise::resolution, 1, bitMap) == 0) {
			std::cerr << "Failed to write noise to file" << std::endl;
		} else {
			std::cout << "Wrote noise to file" << std::endl;
		}

	}


	static const int tableSize = 32;
	static const int resolution = 256;
	float noiseMap[resolution][resolution]{};
	uint8_t bitMap[resolution][resolution]{};
	float table[tableSize][tableSize]{};

};
