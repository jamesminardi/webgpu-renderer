#pragma once

#include <webgpu/webgpu.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "types.h"

class World;
class Chunk;


class TerrainRenderer {


public:

    wgpu::ShaderModule m_shaderModule = nullptr;
    wgpu::BindGroupLayoutDescriptor m_bindGroupLayoutDesc{};
//    wgpu::BindGroup m_bindGroup = nullptr;
    wgpu::BindGroupLayout m_bindGroupLayout = nullptr;
    wgpu::RenderPipeline m_pipeline = nullptr;
	wgpu::RenderPipeline m_wireframePipeline = nullptr;

    ShaderUniforms m_uniforms{};
	wgpu::BufferDescriptor bufferDesc{};




public:

	TerrainRenderer() = default;

    explicit TerrainRenderer(World* w);

	~TerrainRenderer();


	void initChunkBuffers(Chunk& chunk);

	void initChunkUniforms(Chunk& chunk);

	void initChunkBindGroup(Chunk& chunk);


	void update(World& world);

	void render();


	void render(World& world, wgpu::RenderPassEncoder &renderPass);

	void initRenderPipeline();


	void terminateRenderPipeline();


};

