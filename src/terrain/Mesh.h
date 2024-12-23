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

template<uint16_t N>
class Mesh {

public:

	/**
	 * Triangle defined by 3 indices
	 */
	struct Triangle {
		uint16_t a, b, c;
	};

	std::vector<Vertex> vertices;
//	std::vector<uint16_t> indices; // Keep as uint16_t, b/c WebGPU requires multiple of 4, and need to pad.
	std::vector<Triangle> triangles;

	Mesh() = default;

	void clear() {
		vertices.clear();
		triangles.clear();
//		indices.clear();
	}

	void addVertex(const Vertex &vertex) {
		vertices.push_back(vertex);
	}

//	void addIndex(uint16_t index) {
//		indices.push_back(index);
//	}

	void addTriangle(Triangle triangle) {
		triangles.push_back(triangle);
	}

	void addTriangle(uint16_t a, uint16_t b, uint16_t c) {
		triangles.push_back({a, b, c});
	}

private:


};
