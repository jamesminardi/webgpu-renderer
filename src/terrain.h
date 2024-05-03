#pragma once

#include "globals.h"
#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <webgpu/webgpu.hpp>
#include <stb_image_write.h>
#include "noise/noise.h"
#include <unordered_map>
#include <map>
#include <queue>
#include <set>
#include "types.h"

class World;

inline uint64_t key(int i,int j) {return ((uint64_t)i << 32 | j);}

inline uint64_t key(glm::ivec2 v) {return (uint64_t)(((int64_t)v.x << 32) | v.y);}

inline glm::ivec2 unKey(uint64_t k) {return {k >> 32, k & 0xFFFFFFFF};}




class Mesh {
public:

	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices; // Keep as uint16_t, b/c WebGPU requires multiple of 4, and need to pad.

	wgpu::Buffer vertexBuffer = nullptr;
	wgpu::Buffer indexBuffer = nullptr;
	wgpu::Buffer uniformBuffer = nullptr;
	wgpu::BindGroup bindGroup = nullptr;

	ShaderUniforms uniforms{};

	int numSides = 0;
	int vertsPerSide = 0;
	bool validBuffers = false;

	Mesh() = default;

	/**
	 *
	 * @param heightMap 2D height map, where the y value is the height and x and z are world positions
	 * @param numSides Number of triangle sides per side of the mesh (number of vertices per side = numSides + 1)
	 * @param wireFrame
	 */
	Mesh(const std::vector<glm::vec3> &heightMap, int numSides, bool wireFrame = false) {
		numSides = numSides;
		vertsPerSide = numSides + 1;
		vertices.reserve(vertsPerSide * vertsPerSide);
		indices.reserve(numSides * numSides * 6);

		// Generate vertices
		for (int row = 0; row < vertsPerSide; row++) {
			for (int col = 0; col < vertsPerSide; col++) {

				Vertex vertex{};

				// Position
				// ----------------
				vertex.position = heightMap[row * vertsPerSide + col];


				// Normal
				// ------------

				float left;
				float right;
				float up;
				float down;

				if (col == 0) {
					left = heightMap[row * vertsPerSide + numSides].y;
				} else {
					left = heightMap[row * vertsPerSide + col - 1].y;
				}

				if (col == numSides) {
					right = heightMap[row * vertsPerSide].y;
				} else {
					right = heightMap[row * vertsPerSide + col + 1].y;
				}

				if (row == 0) {
					down = heightMap[(numSides) * vertsPerSide + col].y;
				} else {
					down = heightMap[(row - 1) * vertsPerSide + col].y;
				}

				if (row == numSides) {
					up = heightMap[col].y;
				} else {
					up = heightMap[(row + 1) * vertsPerSide + col].y;
				}

				// Create normal from four surrounding vertices.
				// Creates a blurring effect since the height at the current vertex is not considered

				// Alternative method?
				glm::vec3 normal;
				normal.x = left - right;
				normal.z = down - up;
				normal.y = 2.0f;
				normal = glm::normalize(normal);

				vertex.normal = normal; // Default to up


				// Color
				// ----------
				float r = (float) col / (float) numSides;
				float g = (float) row / (float) numSides;
				float b = (1 - g) * (1 - r);  // You can adjust this value as needed
				vertex.color = {r, g, b};

//				if (wireFrame) {
//					Mesh::addLineIndices(indices, vertsPerSide, row, col);
//				} else {
//				}
				Mesh::addTriangleIndices(indices, vertsPerSide, row, col);

				vertices.push_back(vertex);

			}
		}


		if (!wireFrame) {
			std::cout << "Triangle Count: " << indices.size() / 3 << std::endl;
			std::cout << "Vertex Count: " << vertices.size() << std::endl;
			assert(vertsPerSide * vertsPerSide == vertices.size());
			assert(numSides * numSides * 2 == indices.size() / 3);
		}
		else {
//			std::cout << "Line Count: " << indices.size() / 2 << std::endl;
//			std::cout << "Vertex Count: " << vertices.size() << std::endl;
//			assert(vertsPerSide * vertsPerSide == vertices.size());
//			assert(numSides * numSides * 4 == indices.size() / 2);
		}
		// Adjust index data to be a multiple of 4 (required by WebGPU)
		while (indices.size() % 4 != 0) {
			indices.push_back(0);
		}

	}

	~Mesh() {
		if (validBuffers) {
			vertexBuffer.destroy();
			indexBuffer.destroy();
			uniformBuffer.destroy();
			vertexBuffer.release();
			indexBuffer.release();
			uniformBuffer.release();
			bindGroup.release();
		}
	}

private:

	// Add the two triangles of indices associated with the given row and column to the indices vector
	static void addTriangleIndices(std::vector<uint16_t>& indices, int vertsPerSide, int row, int col) {

		// Don't do for last column and row since triangles are made from the left and bottom
		if (row < vertsPerSide - 1 && col < vertsPerSide - 1) {
			// Indices
			// --------------
			uint16_t bottomLeft = row * vertsPerSide + col;
			uint16_t bottomRight = bottomLeft + 1;
			uint16_t topLeft = (row + 1) * vertsPerSide + col;
			uint16_t topRight = topLeft + 1;
			// These go from left to right, bottom to top
			//  _________________________________
			//  | \   2 | \   4 | \     | \     |
			//  |   \   |   \   |   \   |   \   |
			//  |  1  \ |  3  \ | ... \ |     \ |
			//  ---------------------------------
			// (Diagonal from top-left to bottom-right)

			// First triangle
			indices.push_back(bottomLeft);
			indices.push_back(bottomRight);
			indices.push_back(topLeft);

			// Second triangle
			indices.push_back(topLeft);
			indices.push_back(bottomRight);
			indices.push_back(topRight);
		}

	}

	static void addLineIndices(std::vector<uint16_t>& indices, int vertsPerSide, int row, int col) {

		uint16_t bottomLeft = 0;
		uint16_t bottomRight = 0;
		uint16_t topLeft = 0;
		uint16_t topRight = 0;

		// Will always add a bottom line
		indices.push_back(bottomLeft);
		indices.push_back(bottomRight);

		// Only add left and center line if not last row
		if (row < vertsPerSide - 1) {
			// Left Line
			indices.push_back(topLeft);
			indices.push_back(bottomLeft);

			// Center line
			indices.push_back(bottomRight);
			indices.push_back(topLeft);

			// Only add right line at the end of the row, but not at the last row
			if (col == vertsPerSide - 1) {
				// Right Line
				indices.push_back(topRight);
				indices.push_back(bottomRight);
			}

		}

	}

};


class Chunk {
public:

	Chunk() = default;

	Chunk(Noise noise, glm::ivec2 worldPosition, int chunkSize = DefaultChunkSize, bool wireFrame = false) :
			worldPos(worldPosition)
	{
		chunkSeed = noise.desc.seed * worldPos.x + worldPos.y;

		std::vector<glm::vec3> heightMap;
		heightMap.reserve((chunkSize + 1) * (chunkSize + 1));

		for (int row = 0; row <= chunkSize; row++) {

			int worldPosY = row + (worldPos.y * chunkSize);

			for (int col = 0; col <= chunkSize; col++) {

				int worldPosX = col + (worldPos.x * chunkSize);

				float h = noise.eval({worldPosX, worldPosY});
				heightMap.emplace_back(worldPosX, h, worldPosY);

			}
		}

		mesh = Mesh(heightMap, chunkSize, wireFrame);
	}


	static constexpr int DefaultChunkSize = 32;
	int chunkSeed = 0;
	glm::ivec2 worldPos{};
	Mesh mesh;

};


struct PointOfInterest {
	glm::ivec2 center;
	int distance;
};

const auto posCmp2 = [](const std::pair<int, int>& a, const std::pair<int, int>& b) {
	if (a.first != b.first) {
		return a.first < b.first;
	}
	else {
		return a.second < b.second;
	}
};

const auto posCmp = [](const glm::ivec2& a, const glm::ivec2& b) {
	if (a.x != b.x) {
		return a.x < b.x;
	}
	else {
		return a.y < b.y;
	}
};

const auto poiCmp = [](PointOfInterest a, PointOfInterest b) {
	if (a.center != b.center) {
		return posCmp(a.center, b.center);
	}
	return a.distance < b.distance;
};


class ChunkLoadStateManager {
public:

	std::set<glm::ivec2, decltype(posCmp)> chunksToLoad;
	std::set<glm::ivec2, decltype(posCmp)> chunksToUnload;
	std::set<glm::ivec2, decltype(posCmp)> chunksToRender;

	std::set<PointOfInterest, decltype(poiCmp)> pois; // centers


	ChunkLoadStateManager() {
	}


	void addPointOfInterest(PointOfInterest pointOfInterest) {
		pois.insert(pointOfInterest);
	}

	void removePointOfInterest(PointOfInterest pointOfInterest) {
		if (pois.find(pointOfInterest) != pois.end()) {
			pois.erase(pointOfInterest);

			// Remove from chunksToLoad if it exists there
			for (auto currentPos : chunksToLoad) {
				if (std::abs(currentPos.x - pointOfInterest.center.x) <= pointOfInterest.distance && // TODO CHECK <= or <
					std::abs(currentPos.y - pointOfInterest.center.y) <= pointOfInterest.distance) {
					chunksToLoad.erase(currentPos);
				}
			}

			for (auto currentPos : chunksToRender) {
				if (std::abs(currentPos.x - pointOfInterest.center.x) <= pointOfInterest.distance && // TODO CHECK <= or <
					std::abs(currentPos.y - pointOfInterest.center.y) <= pointOfInterest.distance) {
					chunksToRender.erase(currentPos);
				}
			}

		}
	}

	void updateChunkLists() {

		for (auto &p: pois) {

			for (int row = -p.distance; row <= p.distance; row++) {
				for (int col = -p.distance; col <= p.distance; col++) {

					// Check if chunk is already loaded
					glm::ivec2 currentChunkPos = p.center + glm::ivec2{col, row};

					if (chunksToUnload.find(currentChunkPos) != chunksToUnload.end()) { // If Chunk is being unloaded
						chunksToUnload.erase(currentChunkPos); // Remove from unload list
					}

					if (chunksToRender.find(currentChunkPos) ==
						chunksToRender.end()) { // If chunk isn't already being rendered
						chunksToLoad.insert(currentChunkPos); // Prep to load it

					}

				}
			}
		}
	}

};




class Terrain {

public:


	/*
	 * Handles all the chunks in the world.
	 * Stores a list of loaded chunks surrounding the center chunk of interest, typically the user.
	 */

	// Number of chunks visible in each direction from the center chunk.
	// Ex. 3 = 7x7 grid.
	// Ex. 2 = 5x5 grid.
	// Ex. 1 = 3x3 grid.
	// Ex. 0 = 1x1 grid.
	// Total number of chunks = (2 * numVisibleChunks + 1)^2
	static constexpr int DefaultLoadDistance = 0;
	static constexpr glm::ivec2 DefaultCenter = {0, 0};
	ChunkLoadStateManager loadManager;
	bool regenerate = false;

	std::map<glm::ivec2, Chunk, decltype(posCmp)> chunks;


	glm::ivec2 center{};
	ShaderUniforms uniforms{};
//	Chunk chunk;
private:


	Noise noise;
	bool wireFrame{};
	int chunkSize{};
	int numVisibleChunks{};

	wgpu::ShaderModule m_shaderModule = nullptr;
	wgpu::BindGroupLayoutDescriptor m_bindGroupLayoutDesc{};
//    wgpu::BindGroup m_bindGroup = nullptr;
	wgpu::BindGroupLayout m_bindGroupLayout = nullptr;
	wgpu::RenderPipeline m_pipeline = nullptr;
	wgpu::RenderPipeline m_wireframePipeline = nullptr;

	wgpu::BufferDescriptor bufferDesc{};



public:


	Terrain() = default;


	explicit Terrain(Noise::Descriptor noiseDesc, glm::ivec2 centerChunkPos = DefaultCenter, int numVisibleChunks = DefaultLoadDistance, int chunkSize = Chunk::DefaultChunkSize, bool wireFrame = false);

	void load();

	// Updates the visible chunks list based on center position
	void update(glm::ivec2 centerChunkPos);

	void setNoise(Noise::Descriptor noiseDesc);
	void setWireFrame(bool wire);

	bool isWireFrame();

	void createRenderPipelines();
	void terminateRenderPipeline();

	void render(wgpu::RenderPassEncoder &renderPass);


	void initChunkBuffers(Chunk& chunk);

	void initChunkUniforms(Chunk& chunk);

	void initChunkBindGroup(Chunk& chunk);



};