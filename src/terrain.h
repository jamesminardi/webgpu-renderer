#pragma once

#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <webgpu/webgpu.hpp>
#include <stb_image_write.h>



//
//float fbmExpensive(glm::vec2 x, float H) {
//	float t = 0.0f;
//	for (int i = 0; i < 3; i++) {
//		float f = pow(2.0f, float(i));
//		float a = pow(f, -H);
//		t += a * valueNoise(x * f);
//	}
//	return t;
//}
//
//float fbm(glm::vec2 x) {
//	float G = 0.5f;
//	float f = 1.0f;
//	float a = 1.0f;
//	float t = 0.0f;
//
//	// Each octave is twice the frequency of the previous one
//	for (int i = 0; i < 3; i++) {
//		t += a * valueNoise(x * f);
//		f *= 2.0f;
//		a *= G;
//	}
//	return t;
//}




//class ValueNoise1D {
//public:
//	ValueNoise1D(int seed = 0){
//		std::srand(seed);
//		for (int i = 0; i < MaxVertices; i++) {
//			r[i] = std::rand() / float(RAND_MAX);
//		}
//	}
//
//
//	float cosineRemap(const float &a, const float &b, const float &t)
//	{
//		assert(t >= 0 && t <= 1);      //t should be in the range [0:1]
//		float tRemap = (1 - std::cos(t * std::numbers::pi)) * 0.5;  //remap t input value
//		return glm::mix(a, b, tRemap);     //return interpolation of a-b using new t
//	}
//
//	float smoothstepRemap(const float &a, const float &b, const float &t)
//	{
//		float tRemapSmoothstep = t * t * (3 - 2 * t);
//		return glm::mix(a, b, tRemapSmoothstep);
//	}
//
//	float smoothEval2(const float &x) {
//		int xMin = static_cast<int>(x);
//		assert(xMin <= MaxVertices - 1);
//		float t = x - xMin;
//
//		return smoothstepRemap(r[xMin], r[xMin + 1], t);
//	}
//
//	float linearEval1(const float &x) {
//		float i = glm::floor(x);
//		float c = glm::ceil(x);
//		float f = glm::fract(x);
//
//
//		// Hash values
//		float iv = hash({i, 0});
//		float cv = hash({c, 0});
//
//		// Interpolate
//		return std::lerp(iv, cv, f);
//	}
//
//	// Same as linearEval1 but uses the precomputed hash value array
//	float linearEval2(const float &x) {
//		int min = glm::floor(x);
//		int ceil = glm::ceil(x);
//		assert(min <= MaxVertices - 1);
//		float t = glm::fract(x);
//
//		return glm::mix(r[min], r[ceil + 1], t);
//	}
//
//	static const int MaxVertices = 256;
//	float r[MaxVertices];
//
//};


class Mesh  {
public:
	std::vector<float> vertices;
	std::vector<uint16_t> indices;
	std::vector<float> colors;
//	wgpu::Buffer verticesBuffer = nullptr;
//	wgpu::Buffer indicesBuffer = nullptr;

	Mesh() = default;

	Mesh(std::vector<float> vertices, std::vector<uint16_t> indices, std::vector<float> colors = {}) : vertices(vertices), indices(indices), colors(colors) {
		std::cout << "Index Count: " << indices.size() << std::endl;
		std::cout << "Vertex Count: " << (vertices.size() / 3) << std::endl;
		std::cout << "Color Count: " << (colors.size() / 3) << std::endl;
		assert(vertices.size() == colors.size());

		// Adjust index data to be a multiple of 4
		while (indices.size() % 4 != 0) {
			indices.push_back(0);
		}

	}

	/*
	 * Generates a random value between 0 and 1 based on the input vector
	 */
	static float hash(glm::vec2 uv) {
		return glm::fract(glm::sin(glm::dot(uv, glm::vec2(12.9898f, 78.233f))) * 43758.5453f);
	}

	static float valueNoise(glm::vec2 x) {
		glm::vec2 i = glm::floor(x);
		glm::vec2 f = glm::fract(x);

		// Smoothstep
		glm::vec2 u = f * f * (3.0f - 2.0f * f);

		// Hash coordinates
		glm::vec2 a = glm::vec2(i.x, i.y);
		glm::vec2 b = glm::vec2(i.x + 1.0f, i.y);
		glm::vec2 c = glm::vec2(i.x, i.y + 1.0f);
		glm::vec2 d = glm::vec2(i.x + 1.0f, i.y + 1.0f);

		// Hash values
		float v1 = hash(a);
		float v2 = hash(b);
		float v3 = hash(c);
		float v4 = hash(d);

		// Interpolate
		float x1 = glm::mix(v1, v2, u.x);
		float x2 = glm::mix(v3, v4, u.x);
		return glm::mix(x1, x2, u.y);
	}

	// Generates vertices from left to right, bottom to top
	static std::vector<float> generateGridVertices(int numCells, float scale) {
		std::vector<float> vertices;
		float halfSize = numCells / 2.0f;

		for (int i = 0; i <= numCells; i++) {
			for (int j = 0; j <= numCells; j++) {
				float x = (j - halfSize) * scale;
				float z = (i - halfSize) * scale;
				float y = valueNoise(glm::vec2(i, j)) * 5.0f;

				vertices.push_back(x);
				vertices.push_back(y);
				vertices.push_back(z);
			}
		}
		return vertices;
	}

	static std::vector<uint16_t> generateGridIndices(int numCells) {
		std::vector<uint16_t> indices;
		for (int i = 0; i < numCells; ++i) {
			for (int j = 0; j < numCells; ++j) {
				uint16_t bottomLeft = i * (numCells + 1) + j;
				uint16_t bottomRight = bottomLeft + 1;
				uint16_t topLeft = (i + 1) * (numCells + 1) + j;
				uint16_t topRight = topLeft + 1;

				// First triangle (top-left, bottom-left, top-right)
				indices.push_back(bottomLeft);
				indices.push_back(bottomRight);
				indices.push_back(topLeft);

				// Second triangle (top-right, bottom-left, bottom-right)
				indices.push_back(topLeft);
				indices.push_back(bottomRight);
				indices.push_back(topRight);
			}
		}
		return indices;
	}

	static std::vector<uint16_t> generateWireFrameGridIndices(int numCells) {
		std::vector<uint16_t> indices;

		int j = 0;
		int i = 0;
		uint16_t bottomLeft = 0;
		uint16_t bottomRight = 0;
		uint16_t topLeft = 0;
		uint16_t topRight = 0;
		for (i = 0; i < numCells; ++i) {
			for (j = 0; j < numCells; ++j) {
				bottomLeft = i * (numCells + 1) + j;
				bottomRight = bottomLeft + 1;
				topLeft = (i + 1) * (numCells + 1) + j;
				topRight = topLeft + 1;

				// Bottom line
				indices.push_back(bottomLeft);
				indices.push_back(bottomRight);

				// Center line
				indices.push_back(bottomRight);
				indices.push_back(topLeft);

				// Left Line
				indices.push_back(topLeft);
				indices.push_back(bottomLeft);

				// Right Line
				indices.push_back(topRight);
				indices.push_back(bottomRight);
			}

			// Add right most line:
			indices.push_back(bottomRight); // bottomRight
			indices.push_back(topRight); // topRight
		}

		// Add top most line
		i = numCells;
		for (j = 0; j < numCells; j++) {
			bottomLeft = i * (numCells + 1) + j;
			bottomRight = bottomLeft + 1;

			indices.push_back(bottomLeft);
			indices.push_back(bottomRight);
		}
		return indices;
	}

	static std::vector<float> generateGridColors(int numCells) {
		std::vector<float> colors;
		for (int i = 0; i <= numCells; i++) {
			for (int j = 0; j <= numCells; j++) {
				float r = static_cast<float>(j) / numCells;
				float g = static_cast<float>(i) / numCells;
				float b = (1 - g) * (1 - r);  // You can adjust this value as needed

				colors.push_back(r);
				colors.push_back(g);
				colors.push_back(b);
			}
		}
		return colors;
	}

};


class Terrain {
public:

	Mesh mesh;

	Terrain() = default;

	Mesh generateSquareMesh(int numSides, float scale, bool wireFrame = false) {
		std::vector<float> vertices = Mesh::generateGridVertices(numSides, scale);
		std::vector<float> colors = Mesh::generateGridColors(numSides);
		std::vector<uint16_t> indices;

		if (wireFrame) {
			indices = Mesh::generateWireFrameGridIndices(numSides);
		} else {
			indices = Mesh::generateGridIndices(numSides);
		}

		this->wireFrame = wireFrame;
		mesh = Mesh(vertices, indices, colors);
		return mesh;
	}

	bool needs_update = false;
	bool wireFrame = false;

private:


};