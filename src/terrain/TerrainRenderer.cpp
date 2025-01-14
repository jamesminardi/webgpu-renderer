#include "TerrainRenderer.h"

#include "../application.h"

TerrainRenderer::TerrainRenderer(Terrain& terrain) : m_terrain(terrain) {
	createRenderPipelines();

//	m_shaderModule = createShaderModule("terrain.vert.spv");
}

TerrainRenderer::~TerrainRenderer() {
	terminateRenderPipeline();
}


void TerrainRenderer::onNotify(Terrain& source, const RoiManager& data) {
	// We can safely assume the chunks to load aren't already loaded. So we can just create and destroy buffers for
	// the chunks in those lists. Then we can compare our current buffers to the loaded chunks as a failsafe.

	for (auto& pos : data.getChunksToLoad()) {
		// We can assume the chunk is not already loaded. If it is, we're just recreating it anyways

		chunkBuffers.erase(pos); // If it doesn't exist, nothing happens
		// TODO: Seems like a code smell...
		// I shouldn't have to navigate through terrain to get the chunk mesh at pos
		// Meshes are needed for rendering but also stuff like colisions, so not sure where meshes should be stored
		// if not in the chunk class itself
		chunkBuffers.emplace(pos, m_terrain.chunks.at(pos).mesh); // Emplace so destructor isn't called on old copied buffer when moving. TODO RAII ISSUE
	}

	for (auto& pos : data.getChunksToUnload()) {
		chunkBuffers.erase(pos);
	}

	// Failsafe
	// Check that the elements in the chunkBuffers map are the same as the chunks in the terrain
	// If they aren't throw error
	// Assert sizes are the same
	if (chunkBuffers.size() != m_terrain.chunks.size()) {
		throw std::runtime_error("Number of loaded chunk buffers (" + std::to_string(chunkBuffers.size()) + ") does not match the number of loaded chunks (" + std::to_string(m_terrain.chunks.size()) + ")!");
	}
	for (auto& [pos, chunkBuffer] : chunkBuffers) { // Redundant?
		if (!m_terrain.chunks.contains(pos)) {
			throw std::runtime_error("Chunk buffer " + std::to_string(pos.x) + ", " + std::to_string(pos.y) + " does not have a matching loaded chunk!");
		}
	}

}


void TerrainRenderer::setWireFrame(bool wire) {
	if (wire == m_wireFrame) return;
	m_wireFrame = wire;
	// Does terrain need to be regenerated after changing render pipeline used? Dont' think so
}

bool TerrainRenderer::isWireFrame() {
	return m_wireFrame;
}

void TerrainRenderer::createRenderPipelines() {

	std::cout << "Creating shader module..." << std::endl;
	m_shaderModule = Shader::loadShaderModule(*Application::device, RESOURCE_DIR "/shaders/static_triangle.wgsl");
	std::cout << "Shader module: " << m_shaderModule << std::endl;


	std::cout << "Creating render pipeline..." << std::endl;
	wgpu::RenderPipelineDescriptor pipelineDesc{};


	// Vector because there are 3 attributes in separate buffers
	// (As opposed to multiple vertex attributes in one buffer)
	// SIKE they are oe buffer now (so there are 3 attributes in one buffer)
	wgpu::VertexBufferLayout vertexBufferLayout{};

	std::vector<wgpu::VertexAttribute> vertexAttribs(3);

	// Position attribute
	vertexAttribs[0].shaderLocation = 0; // Corresponds to @location(...)
	vertexAttribs[0].format = wgpu::VertexFormat::Float32x3; // size of position, Means vec2<f32> in the shader
	vertexAttribs[0].offset = 0; // Index of the first element

	// Normal attribute
	vertexAttribs[1].shaderLocation = 1; // Corresponds to @location(...)
	vertexAttribs[1].format = wgpu::VertexFormat::Float32x3; // size of normal, Means vec3<f32> in the shader
	vertexAttribs[1].offset = 1 * sizeof(glm::vec3); // Index of the first element

	// Color attribute
	vertexAttribs[2].shaderLocation = 2; // Corresponds to @location(...)
	vertexAttribs[2].format = wgpu::VertexFormat::Float32x3; // size of color, Means vec3<f32> in the shader
	vertexAttribs[2].offset = 2 * sizeof(glm::vec3); // Index of the first element

	// Build vertex buffer layout
	vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;
	vertexBufferLayout.attributeCount = 3;
	vertexBufferLayout.attributes = vertexAttribs.data();
	vertexBufferLayout.arrayStride = sizeof(Vertex); // size of color, since only color attribs in this buffer
	std::cout << "Vertex buffer stride: " << sizeof(Vertex) << std::endl;


	pipelineDesc.vertex.bufferCount = 1; //static_cast<uint32_t>(vertexBufferLayouts.size());
	pipelineDesc.vertex.buffers = &vertexBufferLayout;

	// Vertex Shader
	pipelineDesc.vertex.module = m_shaderModule;
	pipelineDesc.vertex.entryPoint = "vs_main";
	pipelineDesc.vertex.constantCount = 0;
	pipelineDesc.vertex.constants = nullptr;

	// Primitive Assembly & Rasterization
	pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList;
	pipelineDesc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined; // Vertices considered sequentially
	pipelineDesc.primitive.frontFace = wgpu::FrontFace::CCW; // Counter-clockwise vertices are front-facing
	pipelineDesc.primitive.cullMode = wgpu::CullMode::None; // Do not cull any triangles for debugging

	// Fragment Shader
	wgpu::FragmentState fragmentState{};
	pipelineDesc.fragment = &fragmentState;
	fragmentState.module = m_shaderModule;
	fragmentState.entryPoint = "fs_main";
	fragmentState.constantCount = 0;
	fragmentState.constants = nullptr;

	// Blend State
	wgpu::BlendState blendState{};
	blendState.color.srcFactor = wgpu::BlendFactor::SrcAlpha;
	blendState.color.dstFactor = wgpu::BlendFactor::OneMinusSrcAlpha;
	blendState.color.operation = wgpu::BlendOperation::Add;
	blendState.alpha.srcFactor = wgpu::BlendFactor::Zero;
	blendState.alpha.dstFactor = wgpu::BlendFactor::One;
	blendState.alpha.operation = wgpu::BlendOperation::Add;

	wgpu::ColorTargetState colorTargetState{};
	colorTargetState.format = Application::swapChainFormat;
	colorTargetState.blend = &blendState;
	colorTargetState.writeMask = wgpu::ColorWriteMask::All; // Could write to only some channels if we wanted

	// Only one target because our render pass only has one output color attachment.
	fragmentState.targetCount = 1;
	fragmentState.targets = &colorTargetState;

	// Depth & Stencil
	// We set up a depth buffer state for the render pipeline
	wgpu::DepthStencilState depthStencilState = wgpu::Default;
	// Keep a fragment only if its depth is lower than the previously blended one
	depthStencilState.depthCompare = wgpu::CompareFunction::Less;
	// Each time a fragment is blended into the target, we update the value of the Z-buffer
	depthStencilState.depthWriteEnabled = true;
	// Store the format in a variable as later parts of the code depend on it
	depthStencilState.format = Application::depthTextureFormat;
	// Deactivate the stencil altogether
	depthStencilState.stencilReadMask = 0;
	depthStencilState.stencilWriteMask = 0;

	pipelineDesc.depthStencil = &depthStencilState;

	// Multisampling
	pipelineDesc.multisample.count = 1; // Samples per pixel
	pipelineDesc.multisample.mask = 0xFFFFFFFFu; // Default value for mask (all bits on)
	pipelineDesc.multisample.alphaToCoverageEnabled = false; // Irrelevant for count=1

	initBindGroupLayout();

	// Pipeline Layout
	wgpu::PipelineLayoutDescriptor layoutDesc{};
	layoutDesc.label = "Pipeline Layout";
	layoutDesc.bindGroupLayoutCount = 1;
	layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&m_bindGroupLayout;

	pipelineDesc.layout = Application::device->createPipelineLayout(layoutDesc);


	m_pipeline = Application::device->createRenderPipeline(pipelineDesc);
	if (!m_pipeline) {
		throw std::runtime_error("Could not create render pipeline!");
	}
	std::cout << "Render pipeline: " << m_pipeline << std::endl;

	// Wire frame pipelinedesc
	wgpu::RenderPipelineDescriptor wireframePipelineDesc = pipelineDesc;
	wireframePipelineDesc.primitive.topology = wgpu::PrimitiveTopology::LineList;
	m_wireframePipeline = Application::device->createRenderPipeline(wireframePipelineDesc);
	if (!m_wireframePipeline) {
		throw std::runtime_error("Could not create wire frame pipeline!");
	}
	std::cout << "Wireframe Render pipeline: " << m_wireframePipeline << std::endl;

//	initUniforms();
	m_bufferDesc.size = sizeof(ShaderUniforms);
	m_bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
	m_bufferDesc.mappedAtCreation = false;

	m_uniformBuffer = Application::device->createBuffer(m_bufferDesc);
	std::cout << "Uniform Buffer: " << m_uniformBuffer << std::endl;


//	initBindGroup();

	wgpu::BindGroupEntry binding{};
	binding.binding = 0;
	binding.buffer = m_uniformBuffer;
	binding.offset = 0;
	binding.size = sizeof(ShaderUniforms);

	// A bind group contains one or multiple bindings
	wgpu::BindGroupDescriptor bindGroupDesc;
	bindGroupDesc.layout = m_bindGroupLayout;
	bindGroupDesc.entryCount = 1;
	bindGroupDesc.entries = &binding;
	m_bindGroup = Application::device->createBindGroup(bindGroupDesc);
	std::cout << "Bind Group: " << m_bindGroup << std::endl;

	writeUniforms();
}

void TerrainRenderer::terminateRenderPipeline() {
	m_pipeline.release();
	m_wireframePipeline.release();
	m_shaderModule.release();
	m_bindGroupLayout.release();
	m_bindGroup.release();
	m_uniformBuffer.release();
	m_pipeline = nullptr;
	m_wireframePipeline = nullptr;
	m_shaderModule = nullptr;
	m_bindGroupLayout = nullptr;
	m_bindGroup = nullptr;
	m_uniformBuffer = nullptr;

}

void TerrainRenderer::render(wgpu::RenderPassEncoder &renderPass) {

	writeUniforms(); // Could just write when the data changes, although it may just update so frequently that it doesn't matter

	if (m_wireFrame) {
		renderPass.setPipeline(m_wireframePipeline);
	}
	else {
		renderPass.setPipeline(m_pipeline);
	}

	for (auto& [pos, chunkBuffer] : chunkBuffers) {
		renderPass.setVertexBuffer(0, chunkBuffer.getVertexBuffer(), 0, m_terrain.chunks.at(pos).mesh.getVertices().size() * sizeof(Vertex));
		renderPass.setIndexBuffer(chunkBuffer.getIndexBuffer(), wgpu::IndexFormat::Uint16, 0, m_terrain.chunks.at(pos).mesh.getTriangles().size() * 3 * sizeof(uint16_t));
		renderPass.setBindGroup(0, m_bindGroup, 0, nullptr);
		renderPass.drawIndexed(m_terrain.chunks.at(pos).mesh.getTriangles().size() * 3, 1, 0, 0, 0);
	}

}


//void TerrainRenderer::initChunkBuffers(Chunk& chunk) {
//
//	auto chunkPos = chunk.worldPos;
//
//	// Create vertex buffer
//	wgpu::BufferDescriptor vertexBufferDesc{};
//	vertexBufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex;
//	vertexBufferDesc.mappedAtCreation = false;
//
//	wgpu::BufferDescriptor indexBufferDesc{};
//	indexBufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index;
//	indexBufferDesc.mappedAtCreation = false;
//
//	// Create vertex buffer
//	vertexBufferDesc.size = chunk.mesh.getVertices().size() * sizeof(Vertex);
//	chunkBuffers.insert({chunk.worldPos, ChunkBuffer(chunk.mesh)});
//	chunk.mesh.getBuffers().vertexBuffer = Application::device->createBuffer(vertexBufferDesc);
//	// Upload vertex data to vertex buffer
//	Application::queue->writeBuffer(chunk.mesh.getBuffers().vertexBuffer, 0, chunk.mesh.getVertices().data(), vertexBufferDesc.size);
//	std::cout << "Vertex Buffer: " << chunk.mesh.getBuffers().vertexBuffer << std::endl;
//
//	// Create index buffer
//	indexBufferDesc.size = chunk.mesh.getTriangles().size() * 3 * sizeof(uint16_t);
//	chunk.mesh.getBuffers().indexBuffer = Application::device->createBuffer(indexBufferDesc);
//	// Upload index data to index buffer
//	Application::queue->writeBuffer(chunk.mesh.getBuffers().indexBuffer, 0, chunk.mesh.getTriangles().data(), indexBufferDesc.size);
//	std::cout << "Index Buffer: " << chunk.mesh.getBuffers().indexBuffer << std::endl;
//}

void TerrainRenderer::initUniforms() {
//	m_bufferDesc.size = sizeof(ShaderUniforms);
//	m_bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
//	m_bufferDesc.mappedAtCreation = false;
//
//	m_uniformBuffer = Application::device->createBuffer(m_bufferDesc);
//	std::cout << "Uniform Buffer: " << m_uniformBuffer << std::endl;

//	writeUniforms();
}

// TODO Could avoid rewriting the entire uniform buffer and just the data that changed (i.e. when the viewmatrix changes but nothing else)
void TerrainRenderer::writeUniforms() {
	// Check if the uniformbuffer is valid
	if (!m_uniformBuffer) {
		throw std::runtime_error("Uniform buffer is not valid!");
	}
//	std::cout << "Uniform Buffer Map State: " << m_uniformBuffer.getMapState() << std::endl;
	Application::queue->writeBuffer(m_uniformBuffer, 0, &m_uniforms, sizeof(ShaderUniforms));
}

void TerrainRenderer::initBindGroupLayout() {
	// Create binding layout (don't forget to = Default)
	wgpu::BindGroupLayoutEntry bindingLayout = wgpu::Default;
	bindingLayout.binding = 0;
	bindingLayout.visibility = wgpu::ShaderStage::Vertex | wgpu::ShaderStage::Fragment;
	bindingLayout.buffer.type = wgpu::BufferBindingType::Uniform;
	bindingLayout.buffer.minBindingSize = sizeof(ShaderUniforms);

	// Create a bind group layout
	wgpu::BindGroupLayoutDescriptor bindGroupLayoutDesc{};
	bindGroupLayoutDesc.entryCount = 1;
	bindGroupLayoutDesc.entries = &bindingLayout;
	m_bindGroupLayout = Application::device->createBindGroupLayout(bindGroupLayoutDesc);
	std::cout << "Bind Group Layout: " << m_bindGroupLayout << std::endl;
}

void TerrainRenderer::initBindGroup() {
	// Create a binding
//	wgpu::BindGroupEntry binding{};
//	binding.binding = 0;
//	binding.buffer = m_uniformBuffer;
//	binding.offset = 0;
//	binding.size = sizeof(ShaderUniforms);
//
//	// A bind group contains one or multiple bindings
//	wgpu::BindGroupDescriptor bindGroupDesc;
//	bindGroupDesc.layout = m_bindGroupLayout;
//	bindGroupDesc.entryCount = 1;
//	bindGroupDesc.entries = &binding;
//	m_bindGroup = Application::device->createBindGroup(bindGroupDesc);
//	std::cout << "Bind Group: " << m_bindGroup << std::endl;
}
