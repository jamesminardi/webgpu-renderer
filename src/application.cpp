#include "application.h"


#include "window.h"
#include "globals.h"
#include <imgui.h>
#include <backends/imgui_impl_wgpu.h>
#include <backends/imgui_impl_glfw.h>
#include "world.h"
#include "terrain.h"
#include "memory"

std::unique_ptr<wgpu::Device> Application::device = nullptr;
std::unique_ptr<wgpu::Queue> Application::queue = nullptr;

wgpu::TextureFormat Application::swapChainFormat = wgpu::TextureFormat::BGRA8Unorm;
wgpu::TextureFormat Application::depthTextureFormat = wgpu::TextureFormat::Depth24Plus;


Application::Application()  {


	initWindowAndDevice();
	initSwapChain();
	initDepthBuffer();
//	initRenderPipeline();
//	initTexture();
//	initGeometry();
//	initUniforms();
//	initBindGroup();
	initWorld();
	initGui();


//	terrainRenderer = TerrainRenderer();

}

Application::~Application() {

	terminateGui();
	terminateWorld();

//	terminateBindGroup();
//	terminateUniforms();
//	terminateGeometry();
//	terminateTexture();
//	terminateRenderPipeline();
	terminateDepthBuffer();
	terminateSwapChain();
	terminateWindowAndDevice();
}

void Application::initWorld() {
	noiseDesc = Noise::Descriptor();
	// updated
	world = std::make_unique<World>(World());

}

void Application::terminateWorld() {
	world->unload();
}

void Application::onFrame() {

//	world->update();


	// Do nothing, this checks for ongoing asynchronous operations and call their callbacks
	// Be sure to destroy/release buffers that relate to the callbacks outside the main loop.
#ifdef WEBGPU_BACKEND_WGPU
	// Non-standardized behavior: submit empty queue to flush callbacks
    // (wgpu-native also has a device.poll but its API is more complex)
    Application::queue->submit(0, nullptr);
#else
	// Non-standard Dawn way
	// Dawn doesn't check for errors immediately, only on device tick.
	Application::device->tick();
#endif

	Globals::window->inputPollEvents();

	// Update uniform buffer

	// Only update the 1-st float of the buffer
//	Application::queue->writeBuffer(m_uniformBuffer, offsetof(ShaderUniforms, time), &m_uniforms.time, sizeof(ShaderUniforms::time));

	// Rotate model matrix
//	angle1 = m_uniforms.time;
//	R1 = glm::rotate(glm::mat4(1.0), std::fmod(angle1, glm::radians(360.0f)), glm::vec3(0.0, 1.0, 0.0));
//	m_uniforms.modelMatrix = R1 * S * T1;
//	m_queue.writeBuffer(m_uniformBuffer, offsetof(ShaderUniforms, modelMatrix), &m_uniforms.modelMatrix, sizeof(ShaderUniforms::modelMatrix));

//	if (world->dirty)
//	{
//		world->unload();
//		world->load(noiseDesc, 1);
//		terminateGeometry();
//		terminateRenderPipeline();
//		initRenderPipeline();
//		initGeometry();
//	}

	world->update();

//	if (chunk.dirty)
//	{
//		terminateGeometry();
//		initGeometry();
//		chunk.dirty = false;
//	}

	// Get target texture view
	auto nextTexture = m_swapChain.getCurrentTextureView();
	if (!nextTexture) {
		throw std::runtime_error("Could not acquire next swap chain texture!");
	}

	wgpu::CommandEncoderDescriptor commandEncoderDesc{};
	commandEncoderDesc.label = "Command Encoder";
	auto commandEncoder = Application::device->createCommandEncoder(commandEncoderDesc);


	wgpu::RenderPassColorAttachment renderPassColorAttachment{};
	renderPassColorAttachment.view = nextTexture;
	renderPassColorAttachment.resolveTarget = nullptr;
	renderPassColorAttachment.loadOp = wgpu::LoadOp::Clear;
	renderPassColorAttachment.storeOp = wgpu::StoreOp::Store;
	renderPassColorAttachment.clearValue = { 0.7, 0.7, 0.8, 1.0 };


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


	renderPassDesc.depthStencilAttachment = &depthStencilAttachment;
//	renderPassDesc.depthStencilAttachment = nullptr;



	renderPassDesc.timestampWriteCount = 0;
	renderPassDesc.timestampWrites = nullptr;


	// Start of per object specifics?




	wgpu::RenderPassEncoder renderPass = commandEncoder.beginRenderPass(renderPassDesc);


	world->render(renderPass);



	// Select which pipeline to use
//	renderPass.setPipeline(m_pipeline);
//
//    renderPass.setVertexBuffer(0, m_vertexBuffer, 0, world->chunk.mesh.vertices.size() * sizeof(Vertex));
//
//	renderPass.setIndexBuffer(m_indexBuffer, wgpu::IndexFormat::Uint16, 0, world->chunk.mesh.indices.size() * sizeof(uint16_t));
//
//	renderPass.setBindGroup(0, m_bindGroup, 0, nullptr);
//
//	// Draw triangles
//	// We use the `vertexCount` variable instead of hard-coding the vertex count
////	renderPass.draw(m_vertexCount,1,0,0);
//	renderPass.drawIndexed(world->chunk.mesh.indices.size(), 1, 0, 0, 0);

	// We add the GUI drawing commands to the render pass
	updateGui(renderPass);

	renderPass.end();

	// Destroy texture view
	nextTexture.release();

	// Submit GPU commands
	wgpu::CommandBufferDescriptor commandBufferDesc{};
	commandBufferDesc.label = "Command Buffer";
	auto commandBuffer = commandEncoder.finish(commandBufferDesc);
	Application::queue->submit(commandBuffer);



	// Present swap chain
	m_swapChain.present();

}

bool Application::isRunning() {
	return !Globals::window->shouldClose();
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
	Globals::window = std::make_unique<Window>(&windowConfig, this);;

	// Get surface
	// ---------------------------------------------------
	m_surface = Globals::window->getSurface(m_instance);


	// Get adapter
	// ---------------------------------------------------
	std::cout << "Requesting adapter..." << std::endl;
	wgpu::RequestAdapterOptions adapterOpts{};
	adapterOpts.nextInChain = nullptr;
	adapterOpts.compatibleSurface = m_surface;
	adapterOpts.powerPreference = wgpu::PowerPreference::LowPower;

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
	requiredLimits.limits.maxVertexBuffers = 8;
	// Maximum size of a buffer is 6 vertices of 2 float each
	requiredLimits.limits.maxBufferSize = 150000 * sizeof(float);
	// Maximum stride between 2 consecutive vertices in the vertex buffer
	requiredLimits.limits.maxVertexBufferArrayStride = sizeof(Vertex); // Needs to be 5 for imgui

	// Must be set even if we do not use storage or uniform buffers for now
	requiredLimits.limits.minStorageBufferOffsetAlignment = supportedLimits.limits.minStorageBufferOffsetAlignment;
	requiredLimits.limits.minUniformBufferOffsetAlignment = supportedLimits.limits.minUniformBufferOffsetAlignment;
	requiredLimits.limits.maxInterStageShaderComponents = 6; // 6 used by imgui, 3 by us
	requiredLimits.limits.maxBindGroups = 4; // Required to be at least 2 for ImGui

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

	Application::device = std::make_unique<wgpu::Device>(adapter.requestDevice(deviceDesc));
	if (!Application::device) {
		throw std::runtime_error("Could not obtain device!");
	}
	std::cout << "Got device: " << Application::device << std::endl;

	// See what adapter and device limits are
	adapter.getLimits(&supportedLimits);
	std::cout << "adapter.maxVertexAttributes: " << supportedLimits.limits.maxVertexAttributes << std::endl;

	Application::device->getLimits(&supportedLimits);
	std::cout << "device.maxVertexAttributes: " << supportedLimits.limits.maxVertexAttributes << std::endl;


	// Set swapChain format by querying the surface
	// ---------------------------------------------------
	// Need to gamma correct both WPGU and DAWN to match up.
	// This is because Dawn only supports BGRA8Unorm right now
#ifdef WEBGPU_BACKEND_WGPU
	swapChainFormat = m_surface.getPreferredFormat(adapter);
#else
	swapChainFormat = wgpu::TextureFormat::BGRA8Unorm; // Dawn only supports this right now.
#endif
	adapter.release();


	// Error callback for more debug info
	m_errorCallbackHandle = Application::device->setUncapturedErrorCallback([](wgpu::ErrorType type, char const* message) {
		std::cerr << "Device error: type " << type;
		if (message) std::cerr << " (message: " << message << ")";
		std::cerr << std::endl;
	});

#ifdef WEBGPU_BACKEND_DAWN
	m_deviceLostCallbackHandle = Application::device->setDeviceLostCallback([](wgpu::DeviceLostReason type, char const* message) {
		std::cerr << "Device lost: reason " << type;
		if (message) std::cerr << " (message: " << message << ")";
		std::cerr << std::endl;
	});
#endif

	// Get Queue
	// ---------------------------------------------------
	// Can submit commands, or write buffer/texture
	Application::queue = std::make_unique<wgpu::Queue>(Application::device->getQueue());
	std::cout << "Command Queue: " << Application::queue << std::endl;

	// Setup queue work done callback
	auto onQueueWorkDone = [](wgpu::QueueWorkDoneStatus status) {
		std::cout << "Queued work finished with status: " << status << std::endl;
	};
#ifdef WEBGPU_BACKEND_WGPU // Dawn does use hints right now
	Application::queue->onSubmittedWorkDone(onQueueWorkDone);
#else
	m_queueWorkDoneCallbackHandle = Application::queue->onSubmittedWorkDone(0, onQueueWorkDone);
#endif

}

void Application::terminateWindowAndDevice() {
	Application::queue->release();
	Application::device->release();
	m_surface.release();
	m_instance.release();
}

void Application::initSwapChain() {
	std::cout << "Creating swapchain..." << std::endl;

	wgpu::SwapChainDescriptor swapChainDesc{};
	swapChainDesc.nextInChain = nullptr;
	swapChainDesc.label = "Swapchain";
	glm::ivec2 size = Globals::window->getSize();
	swapChainDesc.width = size.x;
	swapChainDesc.height = size.y;
	swapChainDesc.usage = wgpu::TextureUsage::RenderAttachment;
	swapChainDesc.format = Application::swapChainFormat;
	swapChainDesc.presentMode = wgpu::PresentMode::Fifo;

	m_swapChain = Application::device->createSwapChain(m_surface, swapChainDesc);
	if (!m_swapChain) {
		throw std::runtime_error("Could not create swap chain!");
	}
	std::cout << "Swapchain: " << m_swapChain << std::endl;
}

void Application::terminateSwapChain() {
	m_swapChain.release();
}

void Application::initDepthBuffer() {

	depthTextureFormat = wgpu::TextureFormat::Depth24Plus;

	// Create the depth texture
	wgpu::TextureDescriptor depthTextureDesc{};
	depthTextureDesc.dimension = wgpu::TextureDimension::_2D;
	depthTextureDesc.format = Application::depthTextureFormat;
	depthTextureDesc.mipLevelCount = 1;
	depthTextureDesc.sampleCount = 1;
	depthTextureDesc.size = { static_cast<uint32_t>(Globals::window->getWidth()), static_cast<uint32_t>(Globals::window->getHeight()), 1 };
	depthTextureDesc.usage = wgpu::TextureUsage::RenderAttachment;
	depthTextureDesc.viewFormatCount = 1;
	depthTextureDesc.viewFormats = (WGPUTextureFormat*)&depthTextureFormat;
	m_depthTexture = Application::device->createTexture(depthTextureDesc);
	std::cout << "Depth texture: " << m_depthTexture << std::endl;

	// Create the view of the depth texture manipulated by the rasterizer
	wgpu::TextureViewDescriptor depthTextureViewDesc{};
	depthTextureViewDesc.label = "Depth Texture View";
	depthTextureViewDesc.aspect = wgpu::TextureAspect::DepthOnly;
	depthTextureViewDesc.baseArrayLayer = 0;
	depthTextureViewDesc.arrayLayerCount = 1;
	depthTextureViewDesc.baseMipLevel = 0;
	depthTextureViewDesc.mipLevelCount = 1;
	depthTextureViewDesc.dimension = wgpu::TextureViewDimension::_2D;
	depthTextureViewDesc.format = Application::depthTextureFormat;
	m_depthTextureView = m_depthTexture.createView(depthTextureViewDesc);
	std::cout << "Depth texture view: " << m_depthTextureView << std::endl;
}

void Application::terminateDepthBuffer() {
	m_depthTextureView.release();
	m_depthTexture.destroy();
	m_depthTexture.release();
}



void Application::initTexture() {

}

void Application::terminateTexture() {

}




//void Application::initGeometry() {
//
//	std::cout << "Creating geometry..." << std::endl;
//
////	noise = Noise(noiseDesc);
//
////	chunk.load(noise, {0, 0}, noiseDesc.wireFrame);
//
//
//	wgpu::BufferDescriptor bufferDesc{};
//	bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Vertex;
//	bufferDesc.mappedAtCreation = false;
//
//    // WEAVED VERTEX BUFFER
//    bufferDesc.size = world->chunk.mesh.vertices.size() * sizeof(Vertex);
//    m_vertexBuffer = m_device.createBuffer(bufferDesc);
//    // Upload vertex data to vertex buffer
//    m_queue.writeBuffer(m_vertexBuffer, 0, world->chunk.mesh.vertices.data(), bufferDesc.size);
//    std::cout << "Vertex Buffer: " << m_vertexBuffer << std::endl;
//
//
//	// Create index buffer
//	bufferDesc.size = world->chunk.mesh.indices.size() * sizeof(uint16_t);
//	bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Index;
//	m_indexBuffer = m_device.createBuffer(bufferDesc);
//	// Upload index data to index buffer
//	m_queue.writeBuffer(m_indexBuffer, 0, world->chunk.mesh.indices.data(), bufferDesc.size); // Whack ass size because it needs to be a multiple of 4
//	std::cout << "Index Buffer: " << m_indexBuffer << std::endl;
//
//}
//
//void Application::terminateGeometry() {
//
//    m_vertexBuffer.destroy();
//	m_indexBuffer.destroy();
//	m_vertexBuffer.release();
//	m_indexBuffer.release();
//}
//
//void Application::initUniforms() {
//
//	std::cout << "Creating uniforms..." << std::endl;
//
//	// Create uniform buffer
//	wgpu::BufferDescriptor bufferDesc{};
//	bufferDesc.size = sizeof(ShaderUniforms);
//	bufferDesc.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;
//	bufferDesc.mappedAtCreation = false;
//	m_uniformBuffer = m_device.createBuffer(bufferDesc);
//
////	focalPoint = {0.0, 0.0, -2.0};
//
//	// Rotate the object
//	angle1 = 2.0f; // arbitrary time
//
//	// Rotate the view point
//	angle2 = 3.0f * std::numbers::pi / 4.0f;
//
//	ratio = static_cast<float>(Globals::window->getWidth()) / static_cast<float>(Globals::window->getHeight());
//	focalLength = 2.0;
//	near = 0.01f;
//	far = 1000.0f;
//	divider = 1 / (focalLength * (far - near));
//
//
//	// NOTE: WebPGU IS LEFT HANDED (negative Z is forward), and assume Y-UP
//
//	// Model, translates the object relative to the world
////	S = glm::scale(S, glm::vec3(0.5f));
////	T1 = glm::translate(T1, glm::vec3(0.0, 0.0, 0.0));
////	R1 = glm::mat4(1.0);
//	m_uniforms.modelMatrix = T1 * R1 * S;
//
//	m_uniforms.viewMatrix = world->camera.updateViewMatrix();
//
//	// Projection
//	fov = 2 * glm::atan(1 / focalLength);
//	m_uniforms.projectionMatrix = glm::perspective(fov, ratio, near, far);
//
//
//	m_uniforms.time = 1.0f;
//	m_uniforms.color = { 0.5f, 0.6f, 1.0f, 1.0f };
//	m_queue.writeBuffer(m_uniformBuffer, 0, &m_uniforms, sizeof(ShaderUniforms));
//}
//
//void Application::terminateUniforms() {
//	m_uniformBuffer.release();
//}
//
//void Application::initBindGroup() {
//
//	std::cout << "Creating bind group..." << std::endl;
//
//	// Create a binding
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
//	m_bindGroup = m_device.createBindGroup(bindGroupDesc);
//
//
//}
//
//void Application::terminateBindGroup() {
//	m_bindGroup.release();
//}

void Application::initGui() {

	std::cout << "Creating gui..." << std::endl;

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::GetIO();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOther(Globals::window->handle, true);
	ImGui_ImplWGPU_Init(*Application::device, 3, Application::swapChainFormat, Application::depthTextureFormat);
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

	ImGui::Text("Camera:");					// Display some text (you can use a format strings too)
	// Text the camera rotation
	ImGui::Text("Rotation: (%.1f, %.1f)", world->camera.rotation.x, world->camera.rotation.y);
	// Text the camera position
	ImGui::Text("Position: (%.1f, %.1f, %.1f)", world->camera.position.x, world->camera.position.y, world->camera.position.z);

	// Text the camera zoom
	ImGui::Text("Zoom: %.1f", world->camera.zoom);

//	ImGui::Checkbox("Another Window", &show_another_window);

//	if (ImGui::SliderInt("sides", &m_size, 16, 512)) {		// Edit 1 int using a slider
//		chunk.needs_update = true;
//	}
//
//	if (ImGui::SliderFloat("triangle scale", &m_scale, 0.01f, 50.0f)) {		// Edit 1 int using a slider
//		chunk.needs_update = true;
//	}

	bool updateTerrain = false;

	const char* items[] = { "Linear", "SmoothStep", "Smoother", "Cubic" };

	static int noiseFunction = 0;
	if (ImGui::Combo("Function", &noiseFunction, items, 4)) {
		std::cout << "Changing Function" << std::endl;
		switch (noiseFunction) {
		case 0:
			std::cout << "Linear" << std::endl;
			noiseDesc.function = Noise::Function::Value;
			noiseDesc.interpolation = Noise::Interpolation::Linear;
			break;
		case 1:
			std::cout << "Smooth" << std::endl;
			noiseDesc.function = Noise::Function::Value;
			noiseDesc.interpolation = Noise::Interpolation::Smoothstep;
			break;
		case 2:
			std::cout << "Smoother" << std::endl;
			noiseDesc.function = Noise::Function::Value;
			noiseDesc.interpolation = Noise::Interpolation::Smootherstep;
			break;
		case 3:
			std::cout << "Cubic" << std::endl;
			noiseDesc.function = Noise::Function::ValueCubic;
			break;
		}
		updateTerrain = true;
	}


	static bool fbm = false;
	if (ImGui::Checkbox("FBM", &fbm)) {
		if (fbm) {
			noiseDesc.fractal = Noise::Fractal::FBM;
		}
		else {
			noiseDesc.fractal = Noise::Fractal::None;
		}
		updateTerrain = true;
	}

	if (ImGui::SliderFloat("Amplitude", &noiseDesc.amplitude, 0.0f, 500.0f)) {
		updateTerrain = true;
	}

	if (ImGui::SliderFloat("Frequency", &noiseDesc.frequency, 0.01f, 0.2f)) {
		updateTerrain = true;
	}

	if (ImGui::SliderInt("Octaves", &noiseDesc.octaves, 1, 6)) {
		updateTerrain = true;
	}

	if (ImGui::SliderFloat("Lacunarity", &noiseDesc.lacunarity, 0.0f, 5.0f)) {
		updateTerrain = true;
	}

	if (ImGui::SliderFloat("Gain", &noiseDesc.gain, 0.0f, 1.0f)) {
		updateTerrain = true;
	}

	if (ImGui::SliderFloat("Weighted Strength", &noiseDesc.weightedStrength, 0.0f, 1.0f)) {
		updateTerrain = true;
	}

	bool wireFrame = world->terrain->isWireFrame();
	if (ImGui::Checkbox("WireFrame", &wireFrame)) {            // Edit bools storing our window open/close state
		world->terrain->setWireFrame(wireFrame);
	}

    if (updateTerrain) {
        world->terrain->setNoise(noiseDesc);
    }


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
	terminateDepthBuffer();
	terminateSwapChain();
	initSwapChain();
	initDepthBuffer();
	ratio = static_cast<float>(width) / static_cast<float>(height);
}

void Application::onKey([[maybe_unused]] Input::Key key,[[maybe_unused]] Input::Action buttonAction,[[maybe_unused]] bool ctrlKey,[[maybe_unused]] bool shiftKey,[[maybe_unused]] bool altKey) {

}

void Application::onMouseMove([[maybe_unused]] glm::vec2 mousePos,[[maybe_unused]] bool ctrlKey,[[maybe_unused]] bool shiftKey,[[maybe_unused]] bool altKey) {
	if (world->camera.dragState.active) {
		glm::vec2 currentMouse = mousePos;

		float deltaX = (currentMouse.x - world->camera.dragState.startMouse.x) * world->camera.dragState.sensitivity;
		float deltaY = (currentMouse.y - world->camera.dragState.startMouse.y) * world->camera.dragState.sensitivity;
		world->camera.rotation.x = world->camera.dragState.startRotation.x - deltaX;
		world->camera.rotation.y = world->camera.dragState.startRotation.y - deltaY;

		// Clamp to avoid going too far when orbiting up/down
		world->camera.rotation.y = glm::clamp(world->camera.rotation.y, glm::radians(0.1f), glm::radians(179.9f));

		updateViewMatrix();
	}
}

void Application::onMouseButton(Input::MouseButton button, Input::Action buttonAction, [[maybe_unused]] glm::vec2 mousePos, [[maybe_unused]] bool ctrlKey, [[maybe_unused]] bool shiftKey, [[maybe_unused]] bool altKey) {
	if (button == Input::MouseButton::Left) {
		switch (buttonAction) {
			case Input::Action::Press:
				world->camera.dragState.active = true;
				world->camera.dragState.startMouse = mousePos;
				world->camera.dragState.startRotation = world->camera.rotation;
				break;
			case Input::Action::Release:
				world->camera.dragState.active = false;
				break;
			default:
				break;
		}
	}
}

void Application::onScroll(glm::vec2 scrollOffset, [[maybe_unused]] glm::vec2 mousePos, [[maybe_unused]] bool ctrlKey, [[maybe_unused]] bool shiftKey, [[maybe_unused]] bool altKey) {
	world->camera.zoom += world->camera.dragState.scrollSensitivity * static_cast<float>(scrollOffset.y);
	world->camera.zoom = glm::clamp(world->camera.zoom, -50.0f, 2.0f);
	updateViewMatrix();
}

void Application::updateViewMatrix() {
	world->terrain->uniforms.viewMatrix = world->camera.updateViewMatrix();

//	Application::queue->writeBuffer(
//			world->chunk->mesh.uniformBuffer,
//				offsetof(ShaderUniforms, viewMatrix),
//				&world->chunk->mesh.uniforms.viewMatrix,
//				sizeof(ShaderUniforms::viewMatrix)
//		);
	for (auto& [key, chunk] : world->terrain->chunks) {
		chunk.mesh.uniforms.viewMatrix = world->terrain->uniforms.viewMatrix;
		Application::queue->writeBuffer(
				chunk.mesh.uniformBuffer,
				offsetof(ShaderUniforms, viewMatrix),
				&chunk.mesh.uniforms.viewMatrix,
				sizeof(ShaderUniforms::viewMatrix)
		);
	}

}
