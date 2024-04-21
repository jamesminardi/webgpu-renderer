#pragma once

#include <vector>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <webgpu/webgpu.hpp>
#include <stb_image_write.h>
#include <unordered_map>
#include "noise/noise.h"


struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 uv;
};

class Mesh  {
public:

    std::vector<Vertex> vertices;
	std::vector<glm::vec3> normals;
	std::vector<uint16_t> indices; // Keep as uint16_t, b/c WebGPU requires multiple of 4, and need to pad.
	std::vector<glm::vec3> colors;
    bool wireFrame = false;
    int numSides = 0;
    int vertsPerSide = 0;

	Mesh() = default;

    Mesh(const std::vector<float>& heightMap, int numSides, bool wireFrame = false) : wireFrame(wireFrame) {
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
                auto x = (float)col;
                auto z = (float)row;
                float y = heightMap[row * vertsPerSide + col];
                vertex.position = {x, y, z};

                // Normal
                // ------------

                float left;
                float right;
                float up;
                float down;

                if (col == 0) {
                    left = heightMap[row * vertsPerSide + numSides];
                } else {
                    left = heightMap[row * vertsPerSide + col - 1];
                }

                if (col == numSides) {
                    right = heightMap[row * vertsPerSide];
                } else {
                    right = heightMap[row * vertsPerSide + col + 1];
                }

                if (row == 0) {
                    down = heightMap[(numSides) * vertsPerSide + col];
                } else {
                    down = heightMap[(row - 1) * vertsPerSide + col];
                }

                if (row == numSides) {
                    up = heightMap[col];
                } else {
                    up = heightMap[(row + 1) * vertsPerSide + col];
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
                float r = (float)col / (float)numSides;
                float g = (float)row / (float)numSides;
                float b = (1 - g) * (1 - r);  // You can adjust this value as needed
                vertex.color = {r, g, b};

                if (wireFrame) {
                    Mesh::addLineIndices(indices, vertsPerSide, row, col);
                } else {
                    Mesh::addTriangleIndices(indices, vertsPerSide, row, col);
                }

                vertices.push_back(vertex);

            }
        }

        std::cout << "Triangle Count: " << indices.size() / 3 << std::endl;
        std::cout << "Vertex Count: " << vertices.size() << std::endl;
        assert(vertsPerSide * vertsPerSide == vertices.size());
        assert(numSides * numSides * 2 == indices.size() / 3);

        // Adjust index data to be a multiple of 4 (required by WebGPU)
        while (indices.size() % 4 != 0) {
            indices.push_back(0);
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


    void setWireFrame(bool wire) {
        if (wire == wireFrame) return;
        wireFrame = wire;

        indices.clear();
        indices.reserve(numSides * numSides * 6);

        for (int row = 0; row < vertsPerSide; row++) {
            for (int col = 0; col < vertsPerSide; col++) {
                if (wireFrame) {
                    Mesh::addLineIndices(indices, numSides, row, col);
                } else {
                    Mesh::addTriangleIndices(indices, numSides, row, col);
                }
            }
        }

    }



//	static std::vector<uint16_t> generateGridIndices(int numCells) {
//		std::vector<uint16_t> indices;
//		for (int i = 0; i < numCells; ++i) {
//			for (int j = 0; j < numCells; ++j) {
//				// These go from left to right, bottom to top
//				uint16_t bottomLeft = i * (numCells + 1) + j;
//				uint16_t bottomRight = bottomLeft + 1;
//				uint16_t topLeft = (i + 1) * (numCells + 1) + j;
//				uint16_t topRight = topLeft + 1;
//
//				// First triangle (top-left, bottom-left, top-right)
//				indices.push_back(bottomLeft);
//				indices.push_back(bottomRight);
//				indices.push_back(topLeft);
//
//
//				// Second triangle (top-right, bottom-left, bottom-right)
//				indices.push_back(topLeft);
//				indices.push_back(bottomRight);
//				indices.push_back(topRight);
//			}
//		}
//		return indices;
//	}

//	static std::vector<uint16_t> generateWireFrameGridIndices(int numCells) {
//		std::vector<uint16_t> indices;
//
//		int j = 0;
//		int i = 0;
//		uint16_t bottomLeft = 0;
//		uint16_t bottomRight = 0;
//		uint16_t topLeft = 0;
//		uint16_t topRight = 0;
//		for (i = 0; i < numCells; ++i) {
//			for (j = 0; j < numCells; ++j) {
//				bottomLeft = i * (numCells + 1) + j;
//				bottomRight = bottomLeft + 1;
//				topLeft = (i + 1) * (numCells + 1) + j;
//				topRight = topLeft + 1;
//
//				// Bottom line
//				indices.push_back(bottomLeft);
//				indices.push_back(bottomRight);
//
//				// Center line
//				indices.push_back(bottomRight);
//				indices.push_back(topLeft);
//
//				// Left Line
//				indices.push_back(topLeft);
//				indices.push_back(bottomLeft);
//
//				// Right Line
//				indices.push_back(topRight);
//				indices.push_back(bottomRight);
//			}
//
//			// Add right most line:
//			indices.push_back(bottomRight); // bottomRight
//			indices.push_back(topRight); // topRight
//		}
//
//		// Add top most line
//		i = numCells;
//		for (j = 0; j < numCells; j++) {
//			bottomLeft = i * (numCells + 1) + j;
//			bottomRight = bottomLeft + 1;
//
//			indices.push_back(bottomLeft);
//			indices.push_back(bottomRight);
//		}
//		return indices;
//	}

//	static std::vector<glm::vec3> generateGridColors(int numCells) {
//		std::vector<glm::vec3> colors;
//		for (int i = 0; i <= numCells; i++) {
//			for (int j = 0; j <= numCells; j++) {
//				float r = static_cast<float>(j) / numCells;
//				float g = static_cast<float>(i) / numCells;
//				float b = (1 - g) * (1 - r);  // You can adjust this value as needed
//
//				colors.push_back({r, g, b});
//			}
//		}
//		return colors;
//	}

};


class Chunk {
public:

	Chunk() = default;

	Chunk(Noise noise, glm::ivec2 worldPosition, int chunkSize = DefaultChunkSize, bool wireFrame = false) :
        worldPos(worldPosition), chunkSize(chunkSize)
    {
        chunkSeed = noise.desc.seed * worldPos.x + worldPos.y;

        std::vector<float> heightMap;
        heightMap.reserve((chunkSize + 1) * (chunkSize + 1));

        for (int row = 0; row <= chunkSize; row++) {

            int worldPosY = row + (worldPos.y * chunkSize);

            for (int col = 0; col <= chunkSize; col++) {

                int worldPosX = col + (worldPos.x * chunkSize);

                float h = noise.eval({worldPosX, worldPosY});
                heightMap.push_back(h);

            }
        }

        mesh = Mesh(heightMap, chunkSize, wireFrame);
    }


	static constexpr int DefaultChunkSize = 128;
	int chunkSize = DefaultChunkSize; // Number of vertices per side of the chunk
	int chunkSeed = 0;
	glm::ivec2 worldPos{};
	Mesh mesh;

};








class Terrain {
public:


	/*
	 * Handles all the chunks in the world.
	 * Stores a list of loaded chunks surrounding the center chunk of interest, typically the user.
	 */

	// Number of chunks visible in each direction from the center chunk.
	// Ex. 3 = 7x7 grid.
	// Total number of chunks = (2 * numVisibleChunks + 1)^2
	static constexpr int DefaultLoadDistance = 3;

	glm::ivec2 center;
	int numVisibleChunks = DefaultLoadDistance;

	std::unordered_map<uint64_t, Chunk> loadedChunks;

	static inline uint64_t key(int i,int j) {return ((uint64_t)i << 32 | j);}

	static inline uint64_t key(glm::ivec2 v) {return ((uint64_t)v.x << 32 | v.y);}



	Terrain(glm::ivec2 center, int numVisibleChunks = DefaultLoadDistance)
	: center(center), numVisibleChunks(numVisibleChunks) {

		auto noise = Noise(Noise::Descriptor());


		// Generate chunk positions of the chunks surrounding the center chunk
		for (int row = -numVisibleChunks; row <= numVisibleChunks; row++) {
			for (int col = -numVisibleChunks; col <= numVisibleChunks; col++) {
				glm::ivec2 worldPos = center + glm::ivec2{col, row};
				loadedChunks[key(worldPos)] = Chunk(noise, worldPos);
			}
		}
	}

    void update(glm::ivec2 center) {
        updateChunks(center);
    }
private:

    void updateChunks(glm::ivec2 center) {
        if (center == this->center) return;

        // Unload old chunks, load new chunks

        // Unload chunks not in new view
        for (auto it = loadedChunks.begin(); it != loadedChunks.end();) {
            if (std::abs(it->second.worldPos.x - center.x) > numVisibleChunks ||
                std::abs(it->second.worldPos.y - center.y) > numVisibleChunks) {
                it = loadedChunks.erase(it);
            } else {
                it++;
            }
        }

        for (int row = -numVisibleChunks; row <= numVisibleChunks; row++) {
            for (int col = -numVisibleChunks; col <= numVisibleChunks; col++) {
                glm::ivec2 worldPos = center + glm::ivec2{col, row};

                // Only load chunks that are not already loaded
                if (loadedChunks.find(key(worldPos)) == loadedChunks.end()) {
                    loadedChunks[key(worldPos)] = Chunk(Noise(Noise::Descriptor()), worldPos);
                }
            }
        }

        this->center = center;



    }


};