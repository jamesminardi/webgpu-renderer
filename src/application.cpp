#include "application.h"


Application::Application() : m_window(nullptr) {
	initWindowAndDevice();
	initSwapChain();
	initDepthBuffer();
	initRenderPipeline();
	initTexture();
	initGeometry();
	initUniforms();
	initBindGroup();
}

Application::~Application() {
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
	m_window->inputPollEvents();

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
	renderPassColorAttachment.clearValue = { 0.0f, 0.2f, 0.3f, 1.0f };


	wgpu::RenderPassDescriptor renderPassDesc{};
	renderPassDesc.label = "Render Pass";
	renderPassDesc.colorAttachmentCount = 1;
	renderPassDesc.colorAttachments = &renderPassColorAttachment;
	renderPassDesc.depthStencilAttachment = nullptr;
	renderPassDesc.timestampWriteCount = 0;
	renderPassDesc.timestampWrites = nullptr;

	wgpu::RenderPassEncoder renderPass = commandEncoder.beginRenderPass(renderPassDesc);

	// Select which pipeline to use
	renderPass.setPipeline(m_pipeline);

	// Draw triangles
	renderPass.draw(3,1,0,0);

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

#ifdef WEBGPU_BACKEND_DAWN
	// Dawn doesn't check for errors immediately, only on device tick.
	m_device.tick();
#endif

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
	windowConfig.title = "Learn WebGPU";
	windowConfig.width = 640;
	windowConfig.height = 480;
	windowConfig.resizable = true;
	m_window = new Window(&windowConfig, this);

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


	// Get device
	// ---------------------------------------------------
	std::cout << "Requesting device..." << std::endl;
	wgpu::DeviceDescriptor deviceDesc{};
	deviceDesc.nextInChain = nullptr;
	deviceDesc.label = "James Device";
	deviceDesc.requiredFeaturesCount = 0; // Do not require any specific feature
	deviceDesc.requiredLimits = nullptr; // Do not require any specific limit
	deviceDesc.defaultQueue.label = "Default Queue";

	m_device = adapter.requestDevice(deviceDesc);
	if (!m_device) {
		throw std::runtime_error("Could not obtain device!");
	}
	std::cout << "Got device: " << m_device << std::endl;


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
		std::cout << "Device error: type " << type;
		if (message) std::cout << " (message: " << message << ")";
		std::cout << std::endl;
	});

#ifdef WEBGPU_BACKEND_DAWN
	m_deviceLostCallbackHandle = m_device.setDeviceLostCallback([](wgpu::DeviceLostReason type, char const* message) {
		std::cout << "Device lost: reason " << type;
		if (message) std::cout << " (message: " << message << ")";
		std::cout << std::endl;
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

	delete m_window;
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

}

void Application::terminateDepthBuffer() {

}

void Application::initRenderPipeline() {

	std::cout << "Creating shader module..." << std::endl;

	const char* shaderSource = R"(
@vertex
fn vs_main(@builtin(vertex_index) in_vertex_index: u32) -> @builtin(position) vec4<f32> {
	var p = vec2f(0.0, 0.0);
	if (in_vertex_index == 0u) {
		p = vec2f(-0.5, -0.5);
	} else if (in_vertex_index == 1u) {
		p = vec2f(0.5, -0.5);
	} else {
		p = vec2f(0.0, 0.5);
	}
	return vec4f(p, 0.0, 1.0);
}

@fragment
fn fs_main() -> @location(0) vec4f {
    return vec4f(0.0, 0.4, 1.0, 1.0);
}
)";

	// Create shader module and chain WGSL shader code
	wgpu::ShaderModuleDescriptor shaderDesc{};
#ifdef WEBGPU_BACKEND_WGPU // Dawn does use hints right now
	shaderDesc.hintCount = 0;
	shaderDesc.hints = nullptr;
#endif
	wgpu::ShaderModuleWGSLDescriptor shaderCodeDesc{};
	shaderCodeDesc.chain.next = nullptr;
	shaderCodeDesc.chain.sType = wgpu::SType::ShaderModuleWGSLDescriptor;
	shaderDesc.nextInChain = &shaderCodeDesc.chain;

	shaderCodeDesc.code = shaderSource;

	m_shaderModule = m_device.createShaderModule(shaderDesc);
	std::cout << "Shader module: " << m_shaderModule << std::endl;


	std::cout << "Creating render pipeline..." << std::endl;
	wgpu::RenderPipelineDescriptor pipelineDesc{};

	// Vertex Fetch (No input buffer right now)
	pipelineDesc.vertex.bufferCount = 0;
	pipelineDesc.vertex.buffers = nullptr;

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
	pipelineDesc.depthStencil = nullptr; // No depth/stencil for now

	// Multisampling
	pipelineDesc.multisample.count = 1; // Samples per pixel
	pipelineDesc.multisample.mask = 0xFFFFFFFFu; // Default value for mask (all bits on)
	pipelineDesc.multisample.alphaToCoverageEnabled = false; // Irrelevant for count=1

	// Pipeline Layout
	pipelineDesc.layout = nullptr; // No pipeline layout for now

	m_pipeline = m_device.createRenderPipeline(pipelineDesc);
	if (!m_pipeline) {
		throw std::runtime_error("Could not create swap chain!");
	}
	std::cout << "Render pipeline: " << m_pipeline << std::endl;
}

void Application::terminateRenderPipeline() {
	m_pipeline.release();
	m_shaderModule.release();
}

void Application::initTexture() {

}

void Application::terminateTexture() {

}

void Application::initGeometry() {

}

void Application::terminateGeometry() {

}

void Application::initUniforms() {

}

void Application::terminateUniforms() {

}

void Application::initBindGroup() {

}

void Application::terminateBindGroup() {

}


// Input Callbacks
// ---------------

void Application::onResize(int width, int height) {
	terminateSwapChain();
	initSwapChain();
}

void Application::onKey(Input::Key key, Input::Action buttonAction, bool ctrlKey, bool shiftKey, bool altKey) {

}

void Application::onMouseMove(glm::vec2 mousePos, bool ctrlKey, bool shiftKey, bool altKey) {

}

void Application::onMouseClick(Input::MouseButton button, Input::Action buttonAction, glm::vec2 mousePos, bool ctrlKey, bool shiftKey, bool altKey) {

}

void Application::onScroll(glm::vec2 scrollOffset, glm::vec2 mousePos, bool ctrlKey, bool shiftKey, bool altKey) {

}
