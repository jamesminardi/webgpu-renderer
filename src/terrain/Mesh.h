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
#include "HeightMap.h"

// Mesh class that represents a square 3D mesh
class Mesh {

public:

	/**
	 * Buffers for the mesh
	 */
	struct MeshBuffers {
		wgpu::Buffer vertexBuffer = nullptr;
		wgpu::Buffer indexBuffer = nullptr;
		bool valid = false;

		void destroy() {
			vertexBuffer.destroy();
			indexBuffer.destroy();
			valid = false;
		}

		void release() {
			vertexBuffer.release();
			indexBuffer.release();
			valid = false;
		}

//		void init() {
//			wgpu::BufferDescriptor vertexBufferDesc{};
//			vertexBufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex;
//			vertexBufferDesc.mappedAtCreation = false;
//
//			wgpu::BufferDescriptor indexBufferDesc{};
//			indexBufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index;
//			indexBufferDesc.mappedAtCreation = false;
//
//			// Create vertex buffer
//			vertexBufferDesc.size = chunk.mesh.vertices.size() * sizeof(Vertex);
//			chunk.mesh.vertexBuffer = Application::device->createBuffer(vertexBufferDesc);
//			// Upload vertex data to vertex buffer
//			Application::queue->writeBuffer(chunk.mesh.vertexBuffer, 0, chunk.mesh.vertices.data(), vertexBufferDesc.size);
//			std::cout << "Vertex Buffer: " << chunk.mesh.vertexBuffer << std::endl;
//
//			// Create index buffer
//			indexBufferDesc.size = chunk.mesh.indices.size() * sizeof(uint16_t);
//			chunk.mesh.indexBuffer = Application::device->createBuffer(indexBufferDesc);
//			// Upload index data to index buffer
//			Application::queue->writeBuffer(chunk.mesh.indexBuffer, 0, chunk.mesh.indices.data(), indexBufferDesc.size);
//			std::cout << "Index Buffer: " << chunk.mesh.indexBuffer << std::endl;
//		}
	};

	/**
	 * Triangle defined by 3 indices
	 */
	struct Triangle {
		uint16_t a, b, c; // Keep as uint16_t, b/c WebGPU requires multiple of 4, and need to pad.
	};


private:

	std::vector<Vertex> vertices;
	std::vector<Triangle> triangles;
	uint16_t vertsPerSide; // Number of vertices per side of the mesh
	MeshBuffers buffers;


public:

	Mesh() = default;

	void clear() {
		vertices.clear();
		triangles.clear();
		vertsPerSide = 0;
	}

	void releaseBuffers() {
		buffers.release();
	}

	void addVertex(const Vertex &vertex) {
		vertices.push_back(vertex);
	}

	void addTriangle(Triangle triangle) {
		triangles.push_back(triangle);
	}

	void addTriangle(uint16_t a, uint16_t b, uint16_t c) {
		triangles.push_back({a, b, c});
	}

	std::vector<Vertex>& getVertices() {
		return vertices;
	}

	std::vector<Triangle>& getTriangles() {
		return triangles;
	}

	MeshBuffers& getBuffers() {
		return buffers;
	}

	uint16_t getVertsPerSide() {
		return vertsPerSide;
	}


	void print() {
		// Print vertices of xy pairs
		std::cout << "Vertices: " << std::endl;
		for (int i = 0; i < vertices.size(); i++) {
			std::cout << "  Vertex " << i << ": " << vertices[i].position.x << ", " << vertices[i].position.y << ", " << vertices[i].position.z << std::endl;
		}
		// Print triangles
		std::cout << "Triangles: " << std::endl;
		for (int i = 0; i < triangles.size(); i++) {
			std::cout << "  Triangle " << i << ": " << triangles[i].a << ", " << triangles[i].b << ", " << triangles[i].c << std::endl;
		}
	}

	/**
	 * Generate a mesh from a height map and indices map. The mesh's triangle indices are formatted like the following:
	 * _________________________________
	 * | \ N+1 | \ N+3 | \     | \     |
	 * |   \   |   \   |   \   |   \   |
	 * |  N  \ | N+2 \ | ... \ |     \ |
	 * _________________________________
	 * | \   1 | \   3 | \     | \     |
	 * |   \   |   \   |   \   |   \   |
	 * |  0  \ |  2  \ | ... \ |     \ |
	 * ---------------------------------
	 * Row: Bottom to Top, Col: Left to Right
	 * Triangles are ordered counter-clockwise
	 * @tparam 	N				Size of the height map (N x N)
	 * @param 	heightMap		Used to determine the height of the vertices in the mesh
	 * @param 	borderIdxMap	Determines which vertices are part of the mesh. Assumes only border vertices are negaative
	 * @param 	wireFrame 		Whether to generate a wireframe mesh
	 * @return  Mesh
	 */
	template<uint16_t N>
	static Mesh generate(const HeightMap<N>& heightMap, const Grid<int, N>& borderIdxMap, const glm::ivec2 offset = {0,0},const bool wireFrame = false) {
		Mesh mesh;
		for (int row = 0; row < N; row++) {
			for (int col = 0; col < N; col++) {
				int idxMapVal = borderIdxMap(row, col);
				if (idxMapVal >= 0) {

					Vertex vertex;
					vertex.position = {col + offset.x, heightMap(row, col), row + offset.y};
					vertex.normal = heightMap.getNormal(row, col);
					float r = (float) col / N;
					float g = (float) row / N;
					float b = (1 - g) * (1 - r);  // You can adjust this value as needed
					vertex.color = {r, g, b};
					mesh.addVertex(vertex);

					// Add the two triangles of indices associated with the given row and column to the indices vector
					// If borderIdxMap(row, col+1) >0 && (row, col+1)>0 && (row+1, col)>0 && (row+2, col)>0 and row < N - 1 && col < N - 1
					if (row < N - 1 && col < N - 1) {
						if (borderIdxMap(row, col+1) >= 0 && borderIdxMap(row+1, col) >= 0 && borderIdxMap(row+1, col+1) >= 0) {
							// Indices
							// --------------
							uint16_t bottomLeft = borderIdxMap(row, col);
							uint16_t bottomRight = bottomLeft + 1;
							uint16_t topLeft = borderIdxMap(row+1, col);
							uint16_t topRight = topLeft + 1;
							// These go from left to right, bottom to top
							//  _________________________________
							//  | \   2 | \   4 | \     | \     |
							//  |   \   |   \   |   \   |   \   |
							//  |  1  \ |  3  \ | ... \ |     \ |
							//  ---------------------------------
							// (Diagonal from top-left to bottom-right)

							mesh.addTriangle({bottomLeft, bottomRight, topLeft});
							mesh.addTriangle({topLeft, bottomRight, topRight});
						}

					}
				}
			}
		}

		mesh.vertsPerSide = (int)std::sqrt((float)mesh.vertices.size());

		if (!wireFrame) {
			std::cout << "Triangle Count: " << mesh.triangles.size() << std::endl;
			std::cout << "Vertex Count: " << mesh.vertices.size() << std::endl;
			assert((mesh.vertsPerSide-1) * (mesh.vertsPerSide-1) * 2 == mesh.triangles.size());

		}
		else {
//			std::cout << "Line Count: " << indices.size() / 2 << std::endl;
//			std::cout << "Vertex Count: " << vertices.size() << std::endl;
//			assert(vertsPerSide * vertsPerSide == vertices.size());
//			assert(numSides * numSides * 4 == indices.size() / 2);
		}
		// Adjust index data to be a multiple of 4 (required by WebGPU)
		while ((mesh.triangles.size() * 3) % 4 != 0) {
			std::cout << "Adding empty triangle indices for WebGPU alignment..." << std::endl;
			mesh.addTriangle({0, 0, 0});
		}

		return mesh;
	}



	template<uint16_t BorderedSize, uint16_t BorderWidth>
	static Mesh generate(const HeightMap<BorderedSize>& heightMap, const glm::ivec2 offset = {0, 0}, const bool wireFrame = false) {


		Grid<int, BorderedSize> borderIdxMap = Grid<int, BorderedSize>::generateBorderedGrid(1);


		Mesh mesh;
		for (int row = 0; row < BorderedSize; row++) {
			for (int col = 0; col < BorderedSize; col++) {

				int idxMapVal = borderIdxMap(row, col);

				if (idxMapVal >= 0) {

					Vertex vertex;
					vertex.position = {col - BorderWidth + offset.x, heightMap(row, col), row - BorderWidth + offset.y};
					vertex.normal = heightMap.getNormal(row, col);
					float r = (float) vertex.position.x / BorderedSize;
					float g = (float) vertex.position.z / BorderedSize;
					float b = (1 - g) * (1 - r);  // You can adjust this value as needed
					vertex.color = {r, g, b};
					mesh.addVertex(vertex);

					// Add the two triangles of indices associated with the given row and column to the indices vector
					// If borderIdxMap(row, col+1) >0 && (row, col+1)>0 && (row+1, col)>0 && (row+2, col)>0 and row < N - 1 && col < N - 1
					if (row < BorderedSize - 1 && col < BorderedSize - 1) {
						if (borderIdxMap(row, col+1) >= 0 && borderIdxMap(row+1, col) >= 0 && borderIdxMap(row+1, col+1) >= 0) {
							// Indices
							// --------------
							uint16_t bottomLeft = borderIdxMap(row, col);
							uint16_t bottomRight = bottomLeft + 1;
							uint16_t topLeft = borderIdxMap(row+1, col);
							uint16_t topRight = topLeft + 1;
							// These go from left to right, bottom to top
							//  _________________________________
							//  | \   2 | \   4 | \     | \     |
							//  |   \   |   \   |   \   |   \   |
							//  |  1  \ |  3  \ | ... \ |     \ |
							//  ---------------------------------
							// (Diagonal from top-left to bottom-right)

							mesh.addTriangle({bottomLeft, bottomRight, topLeft});
							mesh.addTriangle({topLeft, bottomRight, topRight});
						}

					}
				}
			}
		}

		mesh.vertsPerSide = (int)std::sqrt((float)mesh.vertices.size());

		if (!wireFrame) {
			std::cout << "Triangle Count: " << mesh.triangles.size() << std::endl;
			std::cout << "Vertex Count: " << mesh.vertices.size() << std::endl;
			assert((mesh.vertsPerSide-1) * (mesh.vertsPerSide-1) * 2 == mesh.triangles.size());

		}
		else {
//			std::cout << "Line Count: " << indices.size() / 2 << std::endl;
//			std::cout << "Vertex Count: " << vertices.size() << std::endl;
//			assert(vertsPerSide * vertsPerSide == vertices.size());
//			assert(numSides * numSides * 4 == indices.size() / 2);
		}
		// Adjust index data to be a multiple of 4 (required by WebGPU)
		while ((mesh.triangles.size() * 3) % 4 != 0) {
			std::cout << "Adding empty triangle indices for WebGPU alignment..." << std::endl;
			mesh.addTriangle({0, 0, 0});
		}

		return mesh;
	}


private:


};
