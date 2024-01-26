#include "application.h"

#include <imgui.h>
#include <backends/imgui_impl_wgpu.h>
#include <backends/imgui_impl_glfw.h>
#include <algorithm>


#include "color.h"

Application::Application() : m_window(nullptr) {
	initWindowAndDevice();
	initSwapChain();
	initDepthBuffer();
	initRenderPipeline();
	initTexture();
	initGeometry();
	initUniforms();
	initBindGroup();
	initGui();
}

Application::~Application() {
	terminateGui();
	terminateBindGroup();
	terminateUniforms();
	terminateGeometry();
	terminateTexture();
	terminateRenderPipeline();
	terminateDepthBuffer();
	terminateSwapChain();
	terminateWindowAndDevice();
}

void Application::onFrame() {

	// Do nothing, this checks for ongoing asynchronous operations and call their callbacks
	// Be sure to destroy/release buffers that relate to the callbacks outside the main loop.
#ifdef WEBGPU_BACKEND_WGPU
	// Non-standardized behavior: submit empty queue to flush callbacks
    // (wgpu-native also has a device.poll but its API is more complex)
    m_queue.submit(0, nullptr);
#else
	// Non-standard Dawn way
	// Dawn doesn't check for errors immediately, only on device tick.
	m_device.tick();
#endif

	m_window->inputPollEvents();

	// Update uniform buffer
	m_uniforms.time = static_cast<float>(glfwGetTime()); // glfwGetTime returns a double
	// Only update the 1-st float of the buffer
	m_queue.writeBuffer(m_uniformBuffer, offsetof(ShaderUniforms, time), &m_uniforms.time, sizeof(ShaderUniforms::time));

	// Rotate model matrix
//	angle1 = m_uniforms.time;
//	R1 = glm::rotate(glm::mat4(1.0), std::fmod(angle1, glm::radians(360.0f)), glm::vec3(0.0, 1.0, 0.0));
//	m_uniforms.modelMatrix = R1 * S * T1;
//	m_queue.writeBuffer(m_uniformBuffer, offsetof(ShaderUniforms, modelMatrix), &m_uniforms.modelMatrix, sizeof(ShaderUniforms::modelMatrix));


	// Get target texture view
	auto nextTexture = m_swapChain.getCurrentTextureView();
	if (!nextTexture) {
		throw std::runtime_error("Could not acquire next swap chain texture!");
	}

	wgpu::CommandEncoderDescriptor commandEncoderDesc{};
	commandEncoderDesc.label = "Command Encoder";
	auto commandEncoder = m_device.createCommandEncoder(commandEncoderDesc);


	wgpu::RenderPassColorAttachment renderPassColorAttachment{};
	renderPassColorAttachment.view = nextTexture;
	renderPassColorAttachment.resolveTarget = nullptr;
	renderPassColorAttachment.loadOp = wgpu::LoadOp::Clear;
	renderPassColorAttachment.storeOp = wgpu::StoreOp::Store;
	renderPassColorAttachment.clearValue = { 0.05, 0.05, 0.05, 1.0 };


	wgpu::RenderPassDescriptor renderPassDesc{};
	renderPassDesc.label = "Render Pass";
	renderPassDesc.colorAttachmentCount = 1;
	renderPassDesc.colorAttachments = &renderPassColorAttachment;


	// We now add a depth/stencil attachment:
	wgpu::RenderPassDepthStencilAttachment depthStencilAttachment;
	// The view of the depth texture
	depthStencilAttachment.view = m_depthTextureView;

	// The initial value of the depth buffer, meaning "far"
	depthStencilAttachment.depthClearValue = 1.0f;
	// Operation settings comparable to the color attachment
	depthStencilAttachment.depthLoadOp = wgpu::LoadOp::Clear;
	depthStencilAttachment.depthStoreOp = wgpu::StoreOp::Store;
	// we could turn off writing to the depth buffer globally here
	depthStencilAttachment.depthReadOnly = false;

	// Stencil setup, mandatory but unused
	depthStencilAttachment.stencilClearValue = 0;
#ifdef WEBGPU_BACKEND_WGPU
	depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Clear;
	depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Store;
#else
	depthStencilAttachment.stencilLoadOp = wgpu::LoadOp::Undefined;
	depthStencilAttachment.stencilStoreOp = wgpu::StoreOp::Undefined;
#endif
	depthStencilAttachment.stencilReadOnly = true;


//	renderPassDesc.depthStencilAttachment = &depthStencilAttachment;
	renderPassDesc.depthStencilAttachment = nullptr;



	renderPassDesc.timestampWriteCount = 0;
	renderPassDesc.timestampWrites = nullptr;

	wgpu::RenderPassEncoder renderPass = commandEncoder.beginRenderPass(renderPassDesc);

	// Select which pipeline to use
	renderPass.setPipeline(m_pipeline);

	// Set vertex buffer while encoding the render pass
	renderPass.setVertexBuffer(0, m_positionBuffer, 0, m_positionData.size() * sizeof(float));

	renderPass.setVertexBuffer(1, m_colorBuffer, 0, m_colorData.size() * sizeof(float));

	renderPass.setIndexBuffer(m_indexBuffer, wgpu::IndexFormat::Uint16, 0, m_indexData.size() * sizeof(uint16_t));

	renderPass.setBindGroup(0, m_bindGroup, 0, nullptr);

	// Draw triangles
	// We use the `vertexCount` variable instead of hard-coding the vertex count
//	renderPass.draw(m_vertexCount,1,0,0);
	renderPass.drawIndexed(m_indexCount, 1, 0, 0, 0);

	// We add the GUI drawing commands to the render pass
	updateGui(renderPass);

	renderPass.end();

	// Destroy texture view
	nextTexture.release();

	// Submit GPU commands
	wgpu::CommandBufferDescriptor commandBufferDesc{};
	commandBufferDesc.label = "Command Buffer";
	auto commandBuffer = commandEncoder.finish(commandBufferDesc);
	m_queue.submit(commandBuffer);

	// Present swap chain
	m_swapChain.present();

}

bool Application::isRunning() {
	return !m_window->shouldClose();
}

void Application::initWindowAndDevice() {

	m_instance = createInstance(wgpu::InstanceDescriptor{});
	if (!m_instance) {
		throw std::runtime_error("Could not initialize WebGPU!");
	}

	WindowConfig windowConfig{};
	windowConfig.title = "WebGPU App" " (" + m_platformStr + ")";
	windowConfig.width = 640;
	windowConfig.height = 480;
	windowConfig.resizable = true;
	m_window = std::make_unique<Window>(&windowConfig, this);;

	// Get surface
	// ---------------------------------------------------
	m_surface = m_window->getSurface(m_instance);


	// Get adapter
	// ---------------------------------------------------
	std::cout << "Requesting adapter..." << std::endl;
	wgpu::RequestAdapterOptions adapterOpts{};
	adapterOpts.nextInChain = nullptr;
	adapterOpts.compatibleSurface = m_surface;

	auto adapter = m_instance.requestAdapter(adapterOpts);
	std::cout << "Got adapter: " << adapter << std::endl;

	// Inspect Adapter Features
	std::vector<WGPUFeatureName> features;
	size_t featureCount = adapter.enumerateFeatures(nullptr);
	features.resize(featureCount);
	wgpuAdapterEnumerateFeatures(adapter, features.data());
	std::cout << "Adapter features:" << std::endl;
	// High values are extensions provided by the native implementation
	for (auto f : features) {
		std::cout << " - " << f << std::endl;
	}

	wgpu::SupportedLimits supportedLimits;
	adapter.getLimits(&supportedLimits);

	// Get device
	// ---------------------------------------------------
	std::cout << "Requesting device..." << std::endl;

	// Set required limits for the device.
	wgpu::RequiredLimits requiredLimits = wgpu::Default; // Don't forget to set to default first!
	requiredLimits.limits.maxVertexAttributes = 3; // Imgui uses 3
	requiredLimits.limits.maxVertexBuffers = 2;
	// Maximum size of a buffer is 6 vertices of 2 float each
	requiredLimits.limits.maxBufferSize = 150000 * sizeof(float);
	// Maximum stride between 2 consecutive vertices in the vertex buffer
	requiredLimits.limits.maxVertexBufferArrayStride = 6 * sizeof(float); // Needs to be 5 for imgui

	// Must be set even if we do not use storage or uniform buffers for now
	requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;
	requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
	requiredLimits.limits.maxInterStageShaderComponents = 6; // 6 used by imgui, 3 by us
	requiredLimits.limits.maxBindGroups = 3; // Required to be at least 2 for ImGui

	requiredLimits.limits.maxUniformBuffersPerShaderStage = 1;
	requiredLimits.limits.maxUniformBufferBindingSize = 16 * 4 * sizeof(float);

	// Allow textures up to 2K
	requiredLimits.limits.maxTextureDimension1D = 2048;
	requiredLimits.limits.maxTextureDimension2D = 2048;
	requiredLimits.limits.maxTextureArrayLayers = 1;
	requiredLimits.limits.maxSampledTexturesPerShaderStage = 1;
	requiredLimits.limits.maxSamplersPerShaderStage = 1;


	wgpu::DeviceDescriptor deviceDesc{};
	deviceDesc.nextInChain = nullptr;
	deviceDesc.label = "James Device";
	deviceDesc.requiredFeaturesCount = 0; // Do not require any specific feature
	deviceDesc.requiredLimits = &requiredLimits;
	deviceDesc.defaultQueue.label = "Default Queue";

	m_device = adapter.requestDevice(deviceDesc);
	if (!m_device) {
		throw std::runtime_error("Could not obtain device!");
	}
	std::cout << "Got device: " << m_device << std::endl;

	// See what adapter and device limits are
	adapter.getLimits(&supportedLimits);
	std::cout << "adapter.maxVertexAttributes: " << supportedLimits.limits.maxVertexAttributes << std::endl;

	m_device.getLimits(&supportedLimits);
	std::cout << "device.maxVertexAttributes: " << supportedLimits.limits.maxVertexAttributes << std::endl;


	// Set swapChain format by querying the surface
	// ---------------------------------------------------
	// Need to gamma correct both WPGU and DAWN to match up.
	// This is because Dawn only supports BGRA8Unorm right now
#ifdef WEBGPU_BACKEND_WGPU
	m_swapChainFormat = m_surface.getPreferredFormat(adapter);
#else
	m_swapChainFormat = wgpu::TextureFormat::BGRA8Unorm; // Dawn only supports this right now.
#endif
	adapter.release();


	// Error callback for more debug info
	m_errorCallbackHandle = m_device.setUncapturedErrorCallback([](wgpu::ErrorType type, char const* message) {
		std::cerr << "Device error: type " << type;
		if (message) std::cerr << " (message: " << message << ")";
		std::cerr << std::endl;
	});

#ifdef WEBGPU_BACKEND_DAWN
	m_deviceLostCallbackHandle = m_device.setDeviceLostCallback([](wgpu::DeviceLostReason type, char const* message) {
		std::cerr << "Device lost: reason " << type;
		if (message) std::cerr << " (message: " << message << ")";
		std::cerr << std::endl;
	});
#endif

	// Get Queue
	// ---------------------------------------------------
	// Can submit commands, or write buffer/texture
	m_queue = m_device.getQueue();
	std::cout << "Command Queue: " << m_queue << std::endl;

	// Setup queue work done callback
	auto onQueueWorkDone = [](wgpu::QueueWorkDoneStatus status) {
		std::cout << "Queued work finished with status: " << status << std::endl;
	};
#ifdef WEBGPU_BACKEND_WGPU // Dawn does use hints right now
	m_queue.onSubmittedWorkDone(onQueueWorkDone);
#else
	m_queueWorkDoneCallbackHandle = m_queue.onSubmittedWorkDone(0, onQueueWorkDone);
#endif

}

void Application::terminateWindowAndDevice() {
	m_queue.release();
	m_device.release();
	m_surface.release();
	m_instance.release();
}

void Application::initSwapChain() {
	std::cout << "Creating swapchain..." << std::endl;

	wgpu::SwapChainDescriptor swapChainDesc{};
	swapChainDesc.nextInChain = nullptr;
	swapChainDesc.label = "Swapchain";
	glm::ivec2 size = m_window->getSize();
	swapChainDesc.width = size.x;
	swapChainDesc.height = size.y;
	swapChainDesc.usage = wgpu::TextureUsage::RenderAttachment;
	swapChainDesc.format = m_swapChainFormat;
	swapChainDesc.presentMode = wgpu::PresentMode::Fifo;

	m_swapChain = m_device.createSwapChain(m_surface, swapChainDesc);
	if (!m_swapChain) {
		throw std::runtime_error("Could not create swap chain!");
	}
	std::cout << "Swapchain: " << m_swapChain << std::endl;
}

void Application::terminateSwapChain() {
	m_swapChain.release();
}

void Application::initDepthBuffer() {


	// Create the depth texture
	wgpu::TextureDescriptor depthTextureDesc;
	depthTextureDesc.dimension = wgpu::TextureDimension::_2D;
	depthTextureDesc.format = m_depthTextureFormat;
	depthTextureDesc.mipLevelCount = 1;
	depthTextureDesc.sampleCount = 1;
	depthTextureDesc.size = { 640, 480, 1 };
	depthTextureDesc.usage = wgpu::TextureUsage::RenderAttachment;
	depthTextureDesc.viewFormatCount = 1;
	depthTextureDesc.viewFormats = (WGPUTextureFormat*)&m_depthTextureFormat;
	m_depthTexture = m_device.createTexture(depthTextureDesc);
	std::cout << "Depth texture: " << m_depthTexture << std::endl;

	// Create the view of the depth texture manipulated by the rasterizer
	wgpu::TextureViewDescriptor depthTextureViewDesc;
	depthTextureViewDesc.aspect = wgpu::TextureAspect::DepthOnly;
	depthTextureViewDesc.baseArrayLayer = 0;
	depthTextureViewDesc.arrayLayerCount = 1;
	depthTextureViewDesc.baseMipLevel = 0;
	depthTextureViewDesc.mipLevelCount = 1;
	depthTextureViewDesc.dimension = wgpu::TextureViewDimension::_2D;
	depthTextureViewDesc.format = m_depthTextureFormat;
	m_depthTextureView = m_depthTexture.createView(depthTextureViewDesc);
	std::cout << "Depth texture view: " << m_depthTextureView << std::endl;
}

void Application::terminateDepthBuffer() {
	m_depthTextureView.release();
	m_depthTexture.destroy();
	m_depthTexture.release();
}

void Application::initRenderPipeline() {

	std::cout << "Creating shader module..." << std::endl;
	m_shaderModule = Shader::loadShaderModule(m_device, RESOURCE_DIR "/shaders/static_triangle.wgsl");
	std::cout << "Shader module: " << m_shaderModule << std::endl;


	std::cout << "Creating render pipeline..." << std::endl;
	wgpu::RenderPipelineDescriptor pipelineDesc{};


	// Vector because there are 2 attributes in separate buffers
	// (As opposed to multiple vertex attributes in one buffer)
	std::vector<wgpu::VertexBufferLayout> vertexBufferLayouts(2);

	// Position attribute
	wgpu::VertexAttribute positionAttrib;
	positionAttrib.shaderLocation = 0; // Corresponds to @location(...)
	positionAttrib.format = wgpu::VertexFormat::Float32x3; // size of position, Means vec2<f32> in the shader
	positionAttrib.offset = 0; // Index of the first element
	// Build vertex buffer layout
	vertexBufferLayouts[0].attributeCount = 1;
	vertexBufferLayouts[0].attributes = &positionAttrib;
	vertexBufferLayouts[0].arrayStride = 3 * sizeof(float); // size of position, since only color attribs in this buffer
	vertexBufferLayouts[0].stepMode = wgpu::VertexStepMode::Vertex;


	// Color attribute
	wgpu::VertexAttribute colorAttrib;
	colorAttrib.shaderLocation = 1; // Corresponds to @location(...)
	colorAttrib.format = wgpu::VertexFormat::Float32x3; // size of color, Means vec3<f32> in the shader
	colorAttrib.offset = 0; // Index of the first element
	// Build color buffer layout
	vertexBufferLayouts[1].attributeCount = 1;
	vertexBufferLayouts[1].attributes = &colorAttrib;
	vertexBufferLayouts[1].arrayStride = 3 * sizeof(float); // size of color, since only color attribs in this buffer
	vertexBufferLayouts[1].stepMode = wgpu::VertexStepMode::Vertex;


	pipelineDesc.vertex.bufferCount = static_cast<uint32_t>(vertexBufferLayouts.size());
	pipelineDesc.vertex.buffers = vertexBufferLayouts.data();

	// Vertex Shader
	pipelineDesc.vertex.module = m_shaderModule;
	pipelineDesc.vertex.entryPoint = "vs_main";
	pipelineDesc.vertex.constantCount = 0;
	pipelineDesc.vertex.constants = nullptr;

	// Primitive Assembly & Rasterization
	pipelineDesc.primitive.topology = wgpu::PrimitiveTopology::TriangleList; // Treat each 3 vertices as a triangle
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
	colorTargetState.format = m_swapChainFormat;
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
	wgpu::TextureFormat depthTextureFormat = wgpu::TextureFormat::Depth24Plus;
	depthStencilState.format = depthTextureFormat;
	// Deactivate the stencil alltogether
	depthStencilState.stencilReadMask = 0;
	depthStencilState.stencilWriteMask = 0;

//	pipelineDesc.depthStencil = &depthStencilState;
	pipelineDesc.depthStencil = nullptr;

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
	m_bindGroupLayout = m_device.createBindGroupLayout(bindGroupLayoutDesc);

	// Pipeline Layout
	wgpu::PipelineLayoutDescriptor layoutDesc{};
	layoutDesc.label = "Pipeline Layout";
	layoutDesc.bindGroupLayoutCount = 1;
	layoutDesc.bindGroupLayouts = (WGPUBindGroupLayout*)&m_bindGroupLayout;
	pipelineDesc.layout = m_device.createPipelineLayout(layoutDesc);

	m_pipeline = m_device.createRenderPipeline(pipelineDesc);
	if (!m_pipeline) {
		throw std::runtime_error("Could not create swap chain!");
	}
	std::cout << "Render pipeline: " << m_pipeline << std::endl;
}

void Application::terminateRenderPipeline() {
	m_pipeline.release();
	m_shaderModule.release();
	m_bindGroupLayout.release();
}

void Application::initTexture() {

}

void Application::terminateTexture() {

}


void Application::initGeometry() {

	std::cout << "Creating geometry..." << std::endl;

	m_positionData.clear();
	m_colorData.clear();
	m_indexData.clear();

	const auto radiansPerVertex = static_cast<float>(2.0 * std::numbers::pi / numSides);
	const auto degreesPerVertex = static_cast<float>(360.0 / numSides);
	const float radius = 0.5f;

	m_positionData = {
			-0.5, -0.5,  0.5,
			 0.5, -0.5,  0.5,
			-0.5,  0.5,  0.5,
			 0.5,  0.5,  0.5,
			-0.5, -0.5, -0.5,
			 0.5, -0.5, -0.5,
			-0.5,  0.5, -0.5,
			 0.5,  0.5, -0.5
	   };

	m_colorData = {
			1, 1, 1,
			1, 1, 0,
			1, 0, 1,
			1, 0, 0,
			0, 1, 1,
			0, 1, 0,
			0, 0, 1,
			0, 0, 0
	};

	m_indexData = {
			//Top
			2, 6, 7,
			2, 3, 7,
			//Bottom
			0, 4, 5,
			0, 1, 5,
			//Left
			0, 2, 6,
			0, 4, 6,
			//Right
			1, 3, 7,
			1, 5, 7,
			//Front
			0, 2, 3,
			0, 1, 3,
			//Back
			4, 6, 7,
			4, 5, 7
	};

//	m_positionData.push_back(0.0f);
//	m_positionData.push_back(0.0f);
//	m_colorData.push_back(1.0);
//	m_colorData.push_back(1.0);
//	m_colorData.push_back(1.0);
//	for (int i = 0; i < numSides; i++) {
//		float angle = static_cast<float>(i) * radiansPerVertex;
//		float x = radius * std::cos(angle);
//		float y = radius * std::sin(angle);
//		m_positionData.push_back(x);
//		m_positionData.push_back(y);
//		HSV hsv{};
//		hsv.h = static_cast<float>(i) * degreesPerVertex;
//		RGB rgb = hsv2RGB(hsv);
//		m_colorData.push_back(rgb.r);
//		m_colorData.push_back(rgb.g);
//		m_colorData.push_back(rgb.b);
//	}
//	// Triangle ordering goes as follows for a square:
//	// xab, xbc, xcd, xda
//	// X is the center point and abcd are the corners.
//	for (int i = 0; i < numSides - 1; i++) {
//		m_indexData.push_back(0);
//		m_indexData.push_back(i+1);
//		m_indexData.push_back(i+2);
//	}
//	// Last triangle loops back to the beginning
//	m_indexData.push_back(0);
//	m_indexData.push_back(m_indexData.rbegin()[1]); // reverse order starting at 0 for last element, 1 for second-last
//	m_indexData.push_back(1);


	// Confirm that we have the right number of vertices
	m_indexCount = static_cast<int>(m_indexData.size());
	std::cout << "Index Count: " << m_indexCount << std::endl;
	m_vertexCount = static_cast<int>(m_positionData.size() / 3);
	std::cout << "Vertex Count: " << m_positionData.size() / 3 << std::endl;
	std::cout << "Color Count: " << m_colorData.size() / 3 << std::endl;
	assert(m_vertexCount == static_cast<int>(m_colorData.size() / 3));

	// Adjust index data to be a multiple of 4
	while (m_indexData.size() % 4 != 0) {
		m_indexData.push_back(0);
	}

	// Create position buffer
	wgpu::BufferDescriptor bufferDesc{};
	bufferDesc.size = m_positionData.size() * sizeof(float);
	bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex;
	bufferDesc.mappedAtCreation = false;
	m_positionBuffer = m_device.createBuffer(bufferDesc);

	// Upload pos data to position buffer
	m_queue.writeBuffer(m_positionBuffer, 0, m_positionData.data(), bufferDesc.size);

	std::cout << "Position Buffer: " << m_positionBuffer << std::endl;

	// Create color buffer
	bufferDesc.size = m_colorData.size() * sizeof(float);
	m_colorBuffer = m_device.createBuffer(bufferDesc);

	// Upload color data to color buffer
	m_queue.writeBuffer(m_colorBuffer, 0, m_colorData.data(), bufferDesc.size);

	std::cout << "Color Buffer: " << m_colorBuffer << std::endl;


	bufferDesc.size = m_indexData.size() * sizeof(uint16_t);
	bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index;
	m_indexBuffer = m_device.createBuffer(bufferDesc);

	m_queue.writeBuffer(m_indexBuffer, 0, m_indexData.data(), bufferDesc.size); // Whack ass size because it needs to be a multiple of 4

	std::cout << "Index Buffer: " << m_indexBuffer << std::endl;

}

void Application::terminateGeometry() {
	m_positionBuffer.destroy();
	m_positionBuffer.release();
	m_vertexCount = 0;
}

void Application::initUniforms() {

	std::cout << "Creating uniforms..." << std::endl;

	// Create uniform buffer
	wgpu::BufferDescriptor bufferDesc{};
	bufferDesc.size = sizeof(ShaderUniforms);
	bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
	bufferDesc.mappedAtCreation = false;
	m_uniformBuffer = m_device.createBuffer(bufferDesc);

	ShaderUniforms uniforms{};

//	focalPoint = {0.0, 0.0, -2.0};

	// Rotate the object
	angle1 = 2.0f; // arbitrary time

	// Rotate the view point
	angle2 = 3.0f * std::numbers::pi / 4.0f;

	ratio = static_cast<float>(m_window->getWidth()) / static_cast<float>(m_window->getHeight());
	focalLength = 2.0;
	near = 0.01f;
	far = 100.0f;
	divider = 1 / (focalLength * (far - near));


	// NOTE: WebPGU IS LEFT HANDED (negative Z is forward), and assume Y-UP

	// Model, translates the object relative to the world
	S = glm::scale(S, glm::vec3(0.5f));
	T1 = glm::translate(T1, glm::vec3(0.0, 0.0, 0.0));
	R1 = glm::mat4(1.0);
	uniforms.modelMatrix = T1 * R1 * S;

	m_uniforms.viewMatrix = m_camera.updateViewMatrix();

	// Projection
	fov = 2 * glm::atan(1 / focalLength);
	uniforms.projectionMatrix = glm::perspective(fov, ratio, near, far);


	uniforms.time = 1.0f;
	uniforms.color = { 0.5f, 0.6f, 1.0f, 1.0f };
	m_queue.writeBuffer(m_uniformBuffer, 0, &uniforms, sizeof(ShaderUniforms));
}

void Application::terminateUniforms() {
	m_uniformBuffer.release();
}

void Application::initBindGroup() {

	std::cout << "Creating bind group..." << std::endl;

	// Create a binding
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
	m_bindGroup = m_device.createBindGroup(bindGroupDesc);


}

void Application::terminateBindGroup() {
	m_bindGroup.release();
}

void Application::initGui() {

	std::cout << "Creating gui..." << std::endl;

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::GetIO();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOther(m_window->handle, true);
	ImGui_ImplWGPU_Init(m_device, 3, m_swapChainFormat); // m_depthTextureFormat
}

void Application::terminateGui() {
	ImGui_ImplGlfw_Shutdown();
	ImGui_ImplWGPU_Shutdown();
}

void Application::updateGui(wgpu::RenderPassEncoder renderPass) {
	// Start ImGui frame
	ImGui_ImplWGPU_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// [...] Build our UI
	static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	ImGui::Begin("Render");								// Create a window

//	ImGui::Text("This is some useful text.");					// Display some text (you can use a format strings too)
//	ImGui::Checkbox("Demo Window", &show_demo_window);			// Edit bools storing our window open/close state
//	ImGui::Checkbox("Another Window", &show_another_window);

//	if (ImGui::SliderInt("sides", &numSides, 3, 50)) {		// Edit 1 int using a slider
//		initGeometry();
//	}
//	ImGui::ColorEdit3("clear color", (float*)&clear_color);	// Edit 3 floats representing a color


//	ImGui::Text("Mouse Position: (%.1f,%.1f)", m_mousePos.x, m_mousePos.y);
//	ImGui::Text("Mouse Position NDC: (%.1f,%.1f)", m_mousePosNDC.x, m_mousePosNDC.y);
//	ImGui::ColorButton("Mouse Color", ImVec4(m_mouseColor.r, m_mouseColor.g, m_mouseColor.b, 1.0f));

	ImGui::End();

	// Draw the UI
	ImGui::EndFrame();
	// Convert the UI defined above into low-level draw commands
	ImGui::Render();
	// Execute the low-level draw commands on WebGPU backend
	ImGui_ImplWGPU_RenderDrawData(ImGui::GetDrawData(), renderPass);
}


// Input Callbacks
// ---------------

void Application::onResize([[maybe_unused]] int width, [[maybe_unused]] int height) {
	if (width == 0 || height == 0) { // Used for minimizing window
		return;
	}
	terminateSwapChain();
	initSwapChain();
	ratio = static_cast<float>(width) / static_cast<float>(height);
}

void Application::onKey([[maybe_unused]] Input::Key key,[[maybe_unused]] Input::Action buttonAction,[[maybe_unused]] bool ctrlKey,[[maybe_unused]] bool shiftKey,[[maybe_unused]] bool altKey) {

}

void Application::onMouseMove([[maybe_unused]] glm::vec2 mousePos,[[maybe_unused]] bool ctrlKey,[[maybe_unused]] bool shiftKey,[[maybe_unused]] bool altKey) {
	if (m_camera.dragState.active) {
		glm::vec2 currentMouse = mousePos;
		glm::vec2 delta = (currentMouse - m_camera.dragState.startMouse) * m_camera.dragState.sensitivity;
		m_camera.rotation = m_camera.dragState.startRotation + delta;
		// Clamp to avoid going too far when orbitting up/down
		m_camera.rotation.y = glm::clamp(m_camera.rotation.y, -(float)std::numbers::pi / 2 + 1e-5f, (float)std::numbers::pi / 2 - 1e-5f);
		updateViewMatrix();
	}
}

void Application::onMouseButton(Input::MouseButton button, Input::Action buttonAction, [[maybe_unused]] glm::vec2 mousePos, [[maybe_unused]] bool ctrlKey, [[maybe_unused]] bool shiftKey, [[maybe_unused]] bool altKey) {
	if (button == Input::MouseButton::Left) {
		switch (buttonAction) {
			case Input::Action::Press:
				m_camera.dragState.active = true;
				m_camera.dragState.startMouse = mousePos;
				m_camera.dragState.startRotation = m_camera.rotation;
				break;
			case Input::Action::Release:
				m_camera.dragState.active = false;
				break;
			default:
				break;
		}
	}
}

void Application::onScroll(glm::vec2 scrollOffset, [[maybe_unused]] glm::vec2 mousePos, [[maybe_unused]] bool ctrlKey, [[maybe_unused]] bool shiftKey, [[maybe_unused]] bool altKey) {
	m_camera.zoom += m_camera.dragState.scrollSensitivity * static_cast<float>(scrollOffset.y);
	m_camera.zoom = glm::clamp(m_camera.zoom, -2.0f, 2.0f);
	updateViewMatrix();
}

void Application::updateViewMatrix() {
	m_uniforms.viewMatrix = m_camera.updateViewMatrix();
	m_queue.writeBuffer(
			m_uniformBuffer,
			offsetof(ShaderUniforms, viewMatrix),
			&m_uniforms.viewMatrix,
			sizeof(ShaderUniforms::viewMatrix)
	);
}
