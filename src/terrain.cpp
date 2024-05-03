#include "terrain.h"

#include "application.h"
#include "shader.h"
#include "world.h"

Terrain::Terrain(World* world, Noise::Descriptor noiseDesc, glm::ivec2 centerChunkPos, int numVisibleChunks, int chunkSize, bool wireFrame)
		: world(world), center(centerChunkPos), numVisibleChunks(numVisibleChunks), chunkSize(chunkSize), wireFrame(wireFrame) {



	createRenderPipelines();

	noise = Noise(noiseDesc);


//	loadManager.addPointOfInterest(PointOfInterest{center, numVisibleChunks});

//	chunk = Chunk(noise, center, chunkSize, wireFrame);
//	chunks.insert({center, chunk});



	loadManager.chunksToLoad.insert(center);

//	 loadManager.updateChunkLists();
//
	 for (auto& pos : loadManager.chunksToLoad) {
		chunks.insert({pos, Chunk(noise, pos, chunkSize, wireFrame)});
		initChunkBuffers(chunks.at(pos));
		initChunkUniforms(chunks.at(pos));
		initChunkBindGroup(chunks.at(pos));
		chunks.at(pos).mesh.validBuffers = true;
	}

	loadManager.chunksToLoad.clear();


}

// Updates the visible chunks list based on center position
void Terrain::update(glm::ivec2 centerChunkPos) {

//	if (centerChunkPos != this->center) {
//
//		loadManager.removePointOfInterest(PointOfInterest(this->center, numVisibleChunks));
//		loadManager.addPointOfInterest(PointOfInterest(centerChunkPos, numVisibleChunks));
//
//		this->center = centerChunkPos;
//
//		loadManager.updateChunkLists();
//
//		// Unload old chunks
//		for (auto& pos : loadManager.chunksToUnload) {
//			chunks.erase(pos);
//			loadManager.chunksToUnload.erase(pos);
//		}
//
//		// Load new chunks
//		for (auto& pos : loadManager.chunksToLoad) {
//			chunks.insert({pos, Chunk(noise, pos, chunkSize, wireFrame)});
//			// Cant add to chunks to render until renderer creates buffers
//		}
//
//	}

	if (regenerate) {


		for (auto& [pos, chunk] : chunks) {
			loadManager.chunksToLoad.insert(pos);
		}
		chunks.clear();

		for (auto& pos : loadManager.chunksToLoad) {
			chunks.insert({pos, Chunk(noise, pos, chunkSize, wireFrame)});
			initChunkBuffers(chunks.at(pos));
			initChunkUniforms(chunks.at(pos));
			initChunkBindGroup(chunks.at(pos));
			chunks.at(pos).mesh.validBuffers = true;
			// Cant add to chunks to render until renderer creates buffers
		}

		loadManager.chunksToLoad.clear();
		regenerate = false;
	}




}

void Terrain::setNoise(Noise::Descriptor noiseDesc) {
	noise = Noise(noiseDesc);
	regenerate = true;
}

void Terrain::setWireFrame(bool wire) {
	if (wire == wireFrame) return;
	wireFrame = wire;
	regenerate = true;
}

bool Terrain::isWireFrame() {
	return wireFrame;
}

void Terrain::render(wgpu::RenderPassEncoder &renderPass) {
	renderPass.setPipeline(m_pipeline);

	for (auto& [key, chunk] : chunks) {
		renderPass.setVertexBuffer(0, chunk.mesh.vertexBuffer, 0, chunk.mesh.vertices.size() * sizeof(Vertex));
		renderPass.setIndexBuffer(chunk.mesh.indexBuffer, wgpu::IndexFormat::Uint16, 0, chunk.mesh.indices.size() * sizeof(uint16_t));
		renderPass.setBindGroup(0, chunk.mesh.bindGroup, 0, nullptr);
		renderPass.drawIndexed(chunk.mesh.indices.size(), 1, 0, 0, 0);
	}

}

void Terrain::createRenderPipelines() {

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
	vertexBufferLayout.attributeCount = 3;
	vertexBufferLayout.attributes = vertexAttribs.data();
	vertexBufferLayout.arrayStride = sizeof(Vertex); // size of color, since only color attribs in this buffer
	std::cout << "Vertex buffer stride: " << sizeof(Vertex) << std::endl;

	vertexBufferLayout.stepMode = wgpu::VertexStepMode::Vertex;


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

	// Pipeline Layout
	wgpu::PipelineLayoutDescriptor layoutDesc{};
	layoutDesc.label = "Pipeline Layout";
	layoutDesc.bindGroupLayoutCount = 1;
	layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&m_bindGroupLayout;

	pipelineDesc.layout = Application::device->createPipelineLayout(layoutDesc);


	m_pipeline = Application::device->createRenderPipeline(pipelineDesc);
	if (!m_pipeline) {
		throw std::runtime_error("Could not create swap chain!");
	}
	std::cout << "Render pipeline: " << m_pipeline << std::endl;

	// wire frame pipelinedesc
	wgpu::RenderPipelineDescriptor wireframePipelineDesc = pipelineDesc;
	wireframePipelineDesc.primitive.topology = wgpu::PrimitiveTopology::LineList;
	m_wireframePipeline = Application::device->createRenderPipeline(wireframePipelineDesc);
	if (!m_wireframePipeline) {
		throw std::runtime_error("Could not create swap chain!");
	}
	std::cout << "Wireframe Render pipeline: " << m_wireframePipeline << std::endl;


}

void Terrain::terminateRenderPipeline() {
	m_pipeline.release();
	m_wireframePipeline.release();
	m_shaderModule.release();
	m_bindGroupLayout.release();
}

void Terrain::initChunkBuffers(Chunk& chunk) {
	// Create vertex buffer
	wgpu::BufferDescriptor vertexBufferDesc{};
	vertexBufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex;
	vertexBufferDesc.mappedAtCreation = false;

	wgpu::BufferDescriptor indexBufferDesc{};
	indexBufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index;
	indexBufferDesc.mappedAtCreation = false;

	// Create vertex buffer
	vertexBufferDesc.size = chunk.mesh.vertices.size() * sizeof(Vertex);
	chunk.mesh.vertexBuffer = Application::device->createBuffer(vertexBufferDesc);
	// Upload vertex data to vertex buffer
	Application::queue->writeBuffer(chunk.mesh.vertexBuffer, 0, chunk.mesh.vertices.data(), vertexBufferDesc.size);
	std::cout << "Vertex Buffer: " << chunk.mesh.vertexBuffer << std::endl;

	// Create index buffer
	indexBufferDesc.size = chunk.mesh.indices.size() * sizeof(uint16_t);
	chunk.mesh.indexBuffer = Application::device->createBuffer(indexBufferDesc);
	// Upload index data to index buffer
	Application::queue->writeBuffer(chunk.mesh.indexBuffer, 0, chunk.mesh.indices.data(), indexBufferDesc.size);
	std::cout << "Index Buffer: " << chunk.mesh.indexBuffer << std::endl;
}

void Terrain::initChunkUniforms(Chunk& chunk) {

	bufferDesc.size = sizeof(ShaderUniforms);
	bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
	bufferDesc.mappedAtCreation = false;

	chunk.mesh.uniformBuffer = Application::device->createBuffer(bufferDesc);

	chunk.mesh.uniforms = world->m_uniforms;

	Application::queue->writeBuffer(chunk.mesh.uniformBuffer, 0, &chunk.mesh.uniforms, sizeof(ShaderUniforms));
}

void Terrain::initChunkBindGroup(Chunk& chunk) {
	// Create a binding
	wgpu::BindGroupEntry binding{};
	binding.binding = 0;
	binding.buffer = chunk.mesh.uniformBuffer;
	binding.offset = 0;
	binding.size = sizeof(ShaderUniforms);

	// A bind group contains one or multiple bindings
	wgpu::BindGroupDescriptor bindGroupDesc;
	bindGroupDesc.layout = m_bindGroupLayout;
	bindGroupDesc.entryCount = 1;
	bindGroupDesc.entries = &binding;
	chunk.mesh.bindGroup = Application::device->createBindGroup(bindGroupDesc);
}