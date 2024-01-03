#include "application.h"

#include <imgui.h>
#include <backends/imgui_impl_wgpu.h>
#include <backends/imgui_impl_glfw.h>

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
	wgpu::RequiredLimits requiredLimits = wgpu::Default;
	requiredLimits.limits.maxVertexAttributes = 4;
	requiredLimits.limits.maxVertexBuffers = 1;
	requiredLimits.limits.maxBufferSize = 150000 * sizeof(VertexAttributes);
	requiredLimits.limits.maxVertexBufferArrayStride = sizeof(VertexAttributes);
	requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;
	requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
	requiredLimits.limits.maxInterStageShaderComponents = 8;
	requiredLimits.limits.maxBindGroups = 2; // Required to be at least 2 for ImGui

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

}

void Application::terminateDepthBuffer() {

}

void Application::initRenderPipeline() {

	std::cout << "Creating shader module..." << std::endl;
	m_shaderModule = Shader::loadShaderModule(m_device, RESOURCE_DIR "/shaders/static_triangle.wgsl");
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

void Application::initGui() {
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
	static float f = 0.0f;
	static int counter = 0;
	static bool show_demo_window = true;
	static bool show_another_window = false;
	static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	ImGui::Begin("Hello, world!");								// Create a window called "Hello, world!" and append into it.

	ImGui::Text("This is some useful text.");						// Display some text (you can use a format strings too)
	ImGui::Checkbox("Demo Window", &show_demo_window);			// Edit bools storing our window open/close state
	ImGui::Checkbox("Another Window", &show_another_window);

	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);	// Edit 1 float using a slider from 0.0f to 1.0f
	ImGui::ColorEdit3("clear color", (float*)&clear_color);	// Edit 3 floats representing a color

	if (ImGui::Button("Button"))									// Buttons return true when clicked (most widgets return true when edited/activated)
		counter++;
	ImGui::SameLine();
	ImGui::Text("counter = %d", counter);

	ImGuiIO& io = ImGui::GetIO();
	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
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
	terminateSwapChain();
	initSwapChain();
}

void Application::onKey([[maybe_unused]] Input::Key key,[[maybe_unused]] Input::Action buttonAction,[[maybe_unused]] bool ctrlKey,[[maybe_unused]] bool shiftKey,[[maybe_unused]] bool altKey) {

}

void Application::onMouseMove([[maybe_unused]] glm::vec2 mousePos,[[maybe_unused]] bool ctrlKey,[[maybe_unused]] bool shiftKey,[[maybe_unused]] bool altKey) {

}

void Application::onMouseClick([[maybe_unused]] Input::MouseButton button, [[maybe_unused]] Input::Action buttonAction, [[maybe_unused]] glm::vec2 mousePos, [[maybe_unused]] bool ctrlKey, [[maybe_unused]] bool shiftKey, [[maybe_unused]] bool altKey) {

}

void Application::onScroll([[maybe_unused]] glm::vec2 scrollOffset, [[maybe_unused]] glm::vec2 mousePos, [[maybe_unused]] bool ctrlKey, [[maybe_unused]] bool shiftKey, [[maybe_unused]] bool altKey) {

}
