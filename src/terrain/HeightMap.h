#pragma once

#include "../globals.h"
#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <webgpu/webgpu.hpp>
#include <stb_image_write.h>
#include "../noise/noise.h"
#include <unordered_map>
#include <map>
#include <queue>
#include <set>
#include "../types.h"
#include "Grid.h"

/**
 * Data class for a height map
 *
 * @tparam N size of the height map (N x N)
 */
template <uint16_t N>
class HeightMap {

public:

	float minValue;
	float maxValue;

	Grid<float, N> values;

	HeightMap() = default;

	HeightMap(float min, float max) : minValue(min), maxValue(max) {
		values = Grid<float, N>(0.0f);
	}

	// Copy constructor from components
	HeightMap(const Grid<float, N>& values, float min, float max) : minValue(min), maxValue(max), values(values) { }

	// Move constructor from components
	HeightMap(Grid<float, N>&& values, float min, float max) : minValue(min), maxValue(max), values(std::move(values)) { }

	// Copy constructor from heightmap
	HeightMap(const HeightMap<N>& other) : minValue(other.minValue), maxValue(other.maxValue), values(other.values) { }

	// Move constructor from heightmap
	HeightMap(HeightMap<N>&& other) : minValue(other.minValue), maxValue(other.maxValue), values(std::move(other.values)) { }

	HeightMap<N>& operator=(const HeightMap<N>& rhs) {
		minValue = rhs.minValue;
		maxValue = rhs.maxValue;
		values = rhs.values;
		return *this;
	}

	HeightMap<N>& operator=(HeightMap<N>&& rhs) {
		minValue = rhs.minValue;
		maxValue = rhs.maxValue;
		values = std::move(rhs.values);
		return *this;
	}

	float& operator()(uint16_t row, uint16_t col) {
		return values(row, col);
	}

	float operator()(uint16_t row, uint16_t col) const {
		return values(row, col);
	}

	float& operator()(uint16_t idx) {
		return values(idx);
	}

	float operator()(uint16_t idx) const {
		return values(idx);
	}

	static HeightMap<N> generate(const Noise noise = Noise(), const glm::ivec2 offset = {0,0},
							  const float min = 0.0f, const float max = 1.0f) {
		HeightMap<N> heightMap = HeightMap<N>(min, max);
		for (int row = offset.y; row < (N + offset.y); row++) {
			for (int col = offset.x; col < (N + offset.x); col++) {

				float h = noise.eval({row, col});
				heightMap(row - offset.y, col - offset.x) = h;
			}
		}
		return heightMap;
	}

	Grid<float, N> getNormals() const {
		Grid<glm::vec3, N> normals = Grid<glm::vec3, N*N>(0.0f);
		for (int row = 0; row < N; row++) {
			for (int col = 0; col < N; col++) {
				normals(row, col) = getNormal(row, col);
			}
		}
		return normals;
	}

	glm::vec3 getNormal(const int row, const int col) const {
		float left;
		float right;
		float up;
		float down;

		if (col == 0) {
			left = this->values(row * N + N - 1);
		} else {
			left = this->values(row * N + col - 1);
		}

		if (col == N - 1) {
			right = this->values(row * N);
		} else {
			right = this->values(row * N + col + 1);
		}

		if (row == 0) {
			down = this->values((N - 1) * N + col);
		} else {
			down = this->values((row - 1) * N + col);
		}

		if (row == N - 1) {
			up = this->values(col);
		} else {
			up = this->values((row + 1) * N + col);
		}

		// Create normal from four surrounding vertices.
		// Creates a blurring effect since the height at the current vertex is not considered

		// Alternative method?
		glm::vec3 normal;
		normal.x = left - right;
		normal.z = down - up;
		normal.y = 2.0f;
		normal = glm::normalize(normal);
		return normal;
	}

};

///**
// * Height map generator class
//  *
// * @tparam N size of the height map (N x N)
// */
//template <uint16_t N>
//class HeightMapGenerator {
//
//private:
//	Noise noise;
//	glm::ivec2 offset;
//	HeightMap<N> heightMap;
//
//public:
//
//	static void generate(Noise noise = Noise(), glm::ivec2 offset = {0,0}) {
//
//		for (int row = offset.y; row < (N + offset.y); row++) {
//			for (int col = 0; col < N; col++) {
//
//				float h = noise.eval({col, row});
//				heightMap(col, row) = h;
//			}
//		}
//	}
//
//};