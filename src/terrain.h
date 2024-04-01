#pragma once

#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <webgpu/webgpu.hpp>
#include <stb_image_write.h>
#include "noise/noise.h"




class Mesh  {
public:
	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;
	std::vector<uint16_t> indices;
	std::vector<glm::vec3> colors;
//	wgpu::Buffer verticesBuffer = nullptr;
//	wgpu::Buffer indicesBuffer = nullptr;

	Mesh() = default;

//	Mesh(std::vector<float> vertices, std::vector<uint16_t> indices, std::vector<float> colors = {}) : vertices(vertices), indices(indices), colors(colors) {
//		std::cout << "Index Count: " << indices.size() << std::endl;
//		std::cout << "Vertex Count: " << (vertices.size() / 3) << std::endl;
//		std::cout << "Color Count: " << (colors.size() / 3) << std::endl;
//		assert(vertices.size() == colors.size());
//
//		// Adjust index data to be a multiple of 4
//		while (indices.size() % 4 != 0) {
//			indices.push_back(0);
//		}
//
//	}


	// Generates vertices from left to right, bottom to top
//	static std::vector<float> generateGridVertices(int numCells, float scale) {
//		NoiseTable noise(0);
//
//		std::vector<float> vertices;
//		float halfSize = numCells / 2.0f;
//
//		int stepsPerUnit = numCells / noise.tableSize;
//
//		for (int i = 0; i <= numCells; i++) {
//			for (int j = 0; j <= numCells; j++) {
//				float x = (j - halfSize) * scale;
//				float z = (i - halfSize) * scale;
//
//				float xAdjusted = x / float(stepsPerUnit);
//				float zAdjusted = z / float(stepsPerUnit);
//				float y = noise.evalBicubic(glm::vec2(xAdjusted, zAdjusted)) * 5.0f;
//
//				vertices.push_back(x);
//				vertices.push_back(y);
//				vertices.push_back(z);
//			}
//		}
//		return vertices;
//	}

	static std::vector<glm::vec3> generateGridNormals(int numCells, std::vector<glm::vec3> vertices) {
		std::vector<glm::vec3> normals;
		for (int row = 0; row <= numCells; row++) {
			for (int col = 0; col <= numCells; col++) {

				glm::vec3 left;
				glm::vec3 right;
				glm::vec3 up;
				glm::vec3 down;

				// If the current vertex is on the edge of the grid, wrap around to the opposite side

				if (col == 0) {
					left = vertices[row * (numCells + 1) + numCells];
				} else {
					left = vertices[row * (numCells + 1) + col - 1];
				}

				if (col == numCells) {
					right = vertices[row * (numCells + 1)];
				} else {
					right = vertices[row * (numCells + 1) + col + 1];
				}

				if (row == 0) {
					down = vertices[(numCells) * (numCells + 1) + col];
				} else {
					down = vertices[(row - 1) * (numCells + 1) + col];
				}

				if (row == numCells) {
					up = vertices[col];
				} else {
					up = vertices[(row + 1) * (numCells + 1) + col];
				}

				// Create normal from four surrounding vertices.
				// Creates a blurring effect since the height at the current vertex is not considered
				glm::vec3 normal;

				// Alternative method?
				glm::vec3 normal2;
				normal2.x = left.y - right.y;
				normal2.z = down.y - up.y;
				normal2.y = 2.0f;
				normal2 = glm::normalize(normal2);

				// Alternative method that doesn't work (why?)
//				glm::vec3 cross = glm::cross(right - left, up - down);
//				normal = glm::normalize(cross);

				normals.push_back(normal2);
			}
		}
		return normals;
	}


	static std::vector<uint16_t> generateGridIndices(int numCells) {
		std::vector<uint16_t> indices;
		for (int i = 0; i < numCells; ++i) {
			for (int j = 0; j < numCells; ++j) {
				// These go from left to right, bottom to top
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

	static std::vector<glm::vec3> generateGridColors(int numCells) {
		std::vector<glm::vec3> colors;
		for (int i = 0; i <= numCells; i++) {
			for (int j = 0; j <= numCells; j++) {
				float r = static_cast<float>(j) / numCells;
				float g = static_cast<float>(i) / numCells;
				float b = (1 - g) * (1 - r);  // You can adjust this value as needed

				colors.push_back({r, g, b});
			}
		}
		return colors;
	}

};


class Chunk {
public:

	Chunk() = default;

	explicit Chunk(int chunkSize) : size(chunkSize) {}

	void load(Noise noise, glm::ivec2 worldPosition, bool wire = false) {

		this->worldPos = worldPosition;

		mesh.vertices.resize((size+1) * (size+1));

		chunkSeed = noise.desc.seed * worldPos.x + worldPos.y;

		if (wire) {
			mesh.indices = Mesh::generateWireFrameGridIndices(size);
		} else {
			mesh.indices = Mesh::generateGridIndices(size);
		}

		for (int row = 0; row <= size; row++) {

			int worldPosY = row + (worldPos.y * size);
			for (int col = 0; col <= size; col++) {

				int worldPosX = col + (worldPos.x * size);
				float h = noise.eval({worldPosX, worldPosY});
				mesh.vertices[row * (size+1) + col] = {worldPosX, h, worldPosY};
			}
		}
		mesh.normals = Mesh::generateGridNormals(size, mesh.vertices);
		mesh.indices = Mesh::generateGridIndices(size);
		mesh.colors = Mesh::generateGridColors(size);

		std::cout << "Index Count: " << mesh.indices.size() << std::endl;
		std::cout << "Vertex Count: " << (mesh.vertices.size()) << std::endl;
		std::cout << "Color Count: " << (mesh.colors.size()) << std::endl;
		std::cout << "Normal Count: " << (mesh.normals.size()) << std::endl;
		assert(mesh.vertices.size() == mesh.colors.size());
		assert(mesh.vertices.size() == mesh.normals.size());

		// Adjust index data to be a multiple of 4
		while (mesh.indices.size() % 4 != 0) {
			mesh.indices.push_back(0);
		}
	}

	void unload() {
		mesh.vertices.clear();
		mesh.colors.clear();
		mesh.indices.clear();
		mesh.normals.clear();
	}

	static constexpr int DefaultChunkSize = 32;
	int size = DefaultChunkSize; // Number of vertices per side of the chunk
	int chunkSeed = 0;
	glm::ivec2 worldPos{};
	Mesh mesh;
	bool dirty = false;

};








//class Terrain {
//public:
//
//	Mesh mesh;
//
//	Terrain() = default;
//
//	Mesh generateSquareMesh(int numSides, float scale, bool wireFrame = false) {
//		std::vector<float> vertices;
////		std::vector<float> vertices = Mesh::generateGridVertices(numSides, scale);
//		std::vector<float> colors = Mesh::generateGridColors(numSides);
//		std::vector<uint16_t> indices;
//
//		if (wireFrame) {
//			indices = Mesh::generateWireFrameGridIndices(numSides);
//		} else {
//			indices = Mesh::generateGridIndices(numSides);
//		}
//
//		this->wireFrame = wireFrame;
//		mesh = Mesh(vertices, indices, colors);
//		return mesh;
//	}
//
//	bool needs_update = false;
//	bool wireFrame = false;
//
//private:
//
//
//};