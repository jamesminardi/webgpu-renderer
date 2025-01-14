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
#include "Mesh.h"
#include "HeightMap.h"
#include "Grid.h"
#include "Chunk.h"
#include "RoiManager.h"
#include "../shader.h"
#include "Terrain.h"
#include "Observer.h"
#include "../Application.h"

/**
 * Wrapper for the buffers associated with a chunk.
 * TODO: RAII
 */
class ChunkBuffer {

public:

	ChunkBuffer() : vertexBuffer(nullptr), indexBuffer(nullptr), valid(false) {}
	ChunkBuffer& operator=(const ChunkBuffer& other) = default;
	ChunkBuffer(const ChunkBuffer& other) = default;
	~ChunkBuffer() {
		release();
	}

	explicit ChunkBuffer(const Mesh &mesh) {

		// Create vertex buffer
		wgpu::BufferDescriptor vertexBufferDesc{};
		vertexBufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex;
		vertexBufferDesc.mappedAtCreation = false;

		wgpu::BufferDescriptor indexBufferDesc{};
		indexBufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index;
		indexBufferDesc.mappedAtCreation = false;

		// Create vertex buffer
		vertexBufferDesc.size = mesh.getVertices().size() * sizeof(Vertex);
		vertexBuffer = Application::device->createBuffer(vertexBufferDesc);
		// Upload vertex data to vertex buffer
		Application::queue->writeBuffer(vertexBuffer, 0, mesh.getVertices().data(), vertexBufferDesc.size);
		std::cout << "Vertex Buffer: " << vertexBuffer << std::endl;

		// Create index buffer
		indexBufferDesc.size = mesh.getTriangles().size() * sizeof(Mesh::Triangle);
		indexBuffer = Application::device->createBuffer(indexBufferDesc);
		// Upload index data to index buffer
		Application::queue->writeBuffer(indexBuffer, 0, mesh.getTriangles().data(), indexBufferDesc.size);
		std::cout << "Index Buffer: " << indexBuffer << std::endl;

		valid = true;
	}

	[[nodiscard]] bool isValid() const {
		return valid;
	}

	[[nodiscard]] const wgpu::Buffer& getVertexBuffer() const {
		return vertexBuffer;
	}

	[[nodiscard]] const wgpu::Buffer& getIndexBuffer() const {
		return indexBuffer;
	}


private:

	wgpu::Buffer vertexBuffer = nullptr;
	wgpu::Buffer indexBuffer = nullptr;
	bool valid = false;


	void release() {
		if (!valid) return;
		vertexBuffer.release();
		indexBuffer.release();
		vertexBuffer = nullptr;
		indexBuffer = nullptr;
		valid = false;
	}

};



class TerrainRenderer : public IObserver<Terrain, RoiManager> {

public:
	explicit TerrainRenderer(Terrain& terrain);
	~TerrainRenderer() override;
	void setWireFrame(bool wire);
	bool isWireFrame();
	void createRenderPipelines();
	void terminateRenderPipeline();
	void render(wgpu::RenderPassEncoder &renderPass);
//	void initChunkBuffers(Chunk& chunk);
	void initUniforms();
	void writeUniforms();
	void initBindGroup();
	void onNotify(Terrain& source, const RoiManager& data) override;

	void setUniforms(const ShaderUniforms& uniforms) {
		m_uniforms = uniforms;
//		writeUniforms();
	}

	[[nodiscard]] const ShaderUniforms& getUniforms() const {
		return m_uniforms;
	}

//private:
	std::map<glm::ivec2, ChunkBuffer, decltype(posCmp)> chunkBuffers;
	Terrain& m_terrain;
	ShaderUniforms 					m_uniforms{};
	wgpu::ShaderModule 				m_shaderModule = nullptr;
	wgpu::Buffer 					m_uniformBuffer = nullptr;
	wgpu::BindGroupLayoutDescriptor m_bindGroupLayoutDesc{};
	wgpu::BindGroup 				m_bindGroup = nullptr;
	wgpu::BindGroupLayout 			m_bindGroupLayout = nullptr;
	wgpu::RenderPipeline 			m_pipeline = nullptr;
	wgpu::RenderPipeline 			m_wireframePipeline = nullptr;
	wgpu::BufferDescriptor 			m_bufferDesc{};
	bool m_wireFrame = false;


	void initBindGroupLayout();



};
