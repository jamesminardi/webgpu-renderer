#include "glfw3webgpu.h"
#include "GLFW/glfw3.h"

#define WEBGPU_CPP_IMPLEMENTATION
#include "webgpu/webgpu.hpp"

#include <iostream>
#include <vector>

#include "application.h"



int main (int, char**) {

	Application app;
	if (!app.onInit()) return 1;

	while (app.isRunning()) {
		app.onFrame();
	}

	app.onFinish();
	return 0;

	// We create the equivalent of the navigator.gpu if this were web code (instance)
	const wgpu::InstanceDescriptor instanceDesc{};
	auto instance = wgpu::createInstance(instanceDesc);
	if (!instance) {
		std::cerr << "Could not initialize WebGPU!" << std::endl;
		return 1;
	}

	if (!glfwInit()) {
		std::cerr << "Could not initialize GLFW!" << std::endl;
		return 1;
	}

	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Not dealing with resizing for now
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Tell GLFW to not set up any API specific stuff
	GLFWwindow* window = glfwCreateWindow(640, 480, "Learn WebGPU", nullptr, nullptr);
	if (!window) {
		std::cerr << "Could not open window!" << std::endl;
		glfwTerminate();
		return 1;
	}


	// Get the adapter
	std::cout << "Requesting adapter..." << std::endl;
	wgpu::Surface surface = glfwGetWGPUSurface(instance, window);
	surface.
	wgpu::RequestAdapterOptions adapterOpts{};
	adapterOpts.nextInChain = nullptr;
	adapterOpts.compatibleSurface = surface;

	auto adapter = instance.requestAdapter(adapterOpts);
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
	std::cout << "Requesting device..." << std::endl;
	wgpu::DeviceDescriptor deviceDesc{};
	deviceDesc.nextInChain = nullptr;
	deviceDesc.label = "James Device";
	deviceDesc.requiredFeaturesCount = 0; // Do not require any specific feature
	deviceDesc.requiredLimits = nullptr; // Do not require any specific limit
	deviceDesc.defaultQueue.label = "Default Queue";

	auto device = adapter.requestDevice(deviceDesc);
	std::cout << "Got device: " << device << std::endl;

	// Error callback for more debug info
	auto h = device.setUncapturedErrorCallback([](wgpu::ErrorType type, char const* message) {
		std::cerr << "Device Error: type " << type;
		if (message) std::cerr << " (message: " << message << ")";
		std::cerr << std::endl;
	});

	// Get queue
	// Can submit commands, or write buffer/texture
	auto queue = device.getQueue();
	std::cout << "Command Queue: " << queue << std::endl;

	// Setup queue work done callback
#ifdef WEBGPU_BACKEND_WGPU // Dawn does use hints right now
	auto onQueueWorkDone = [](wgpu::QueueWorkDoneStatus status) {
		std::cout << "Queued work finished with status: " << status << std::endl;
	};
	queue.onSubmittedWorkDone(onQueueWorkDone);
#else
	auto onQueueWorkDone = [](wgpu::QueueWorkDoneStatus status) {
		std::cout << "Queued work finished with status: " << status << std::endl;
	};
	queue.onSubmittedWorkDone(0, onQueueWorkDone);
#endif


	// Create swapchain
	std::cout << "Creating swapchain..." << std::endl;

	// Need to gamma correct both WPGU and DAWN to match up.
	// This is because Dawn only supports BGRA8Unorm right now
#ifdef WEBGPU_BACKEND_WGPU
	auto swapChainFormat = surface.getPreferredFormat(adapter);
#else
	auto swapChainFormat = wgpu::TextureFormat::BGRA8Unorm; // Dawn only supports this right now.
#endif

	wgpu::SwapChainDescriptor swapChainDesc{};
	swapChainDesc.nextInChain = nullptr;
	swapChainDesc.label = "Swapchain";
	swapChainDesc.width = 640;
	swapChainDesc.height = 480;
	swapChainDesc.usage = wgpu::TextureUsage::RenderAttachment;
	swapChainDesc.format = swapChainFormat;
	swapChainDesc.presentMode = wgpu::PresentMode::Fifo;

	auto swapChain = device.createSwapChain(surface, swapChainDesc);
	std::cout << "Swapchain: " << swapChain << std::endl;


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

	auto shaderModule = device.createShaderModule(shaderDesc);
	std::cout << "Shader module: " << shaderModule << std::endl;


	std::cout << "Creating render pipeline..." << std::endl;
	wgpu::RenderPipelineDescriptor pipelineDesc{};

	// Vertex Fetch (No input buffer right now)
	pipelineDesc.vertex.bufferCount = 0;
	pipelineDesc.vertex.buffers = nullptr;

	// Vertex Shader
	pipelineDesc.vertex.module = shaderModule;
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
	fragmentState.module = shaderModule;
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
	colorTargetState.format = swapChainFormat;
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

	auto pipeline = device.createRenderPipeline(pipelineDesc);
	std::cout << "Render pipeline: " << pipeline << std::endl;


	// Render Loop
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		// Get target texture view

		auto nextTexture = swapChain.getCurrentTextureView();
		if (!nextTexture) {
			std::cerr << "Could not acquire next swap chain texture!" << std::endl;
			break;
		}

		wgpu::CommandEncoderDescriptor commandEncoderDesc{};
		commandEncoderDesc.label = "Command Encoder";
		auto commandEncoder = device.createCommandEncoder(commandEncoderDesc);


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
		renderPass.setPipeline(pipeline);

		// Draw triangles
		renderPass.draw(3,1,0,0);

		renderPass.end();

		// Destroy texture view
		nextTexture.release();

		// Submit GPU commands
		wgpu::CommandBufferDescriptor commandBufferDesc{};
		commandBufferDesc.label = "Command Buffer";
		auto commandBuffer = commandEncoder.finish(commandBufferDesc);
		queue.submit(commandBuffer);

		// Present swap chain
		swapChain.present();



	}

	// Clean up the WebGPU instance
	pipeline.release();
	shaderModule.release();
	swapChain.release();
	device.release();
	adapter.release();
	instance.release();
	wgpuSurfaceRelease(surface);
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}