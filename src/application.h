#pragma once

#include <webgpu/webgpu.hpp>
#include <glm/glm.hpp>
#include "window.h"
#include "input.h"

class Application {
public:

	Application();

	bool onInit();

	void onFrame();

	void onFinish();

	bool isRunning();

	// Modifier parameters are true if they are pressed while onKey is called
	// TODO: Just have implementation call for modifier keys and mouse position using window get methods instead of passing in
	void onResize(int width, int height);
	void onKey(Input::Key key, Input::Action buttonAction, bool ctrlKey, bool shiftKey, bool altKey);
	void onMouseMove(glm::vec2 mousePos, bool ctrlKey, bool shiftKey, bool altKey);
	void onMouseClick(Input::MouseButton button, Input::Action buttonAction, glm::vec2 mousePos, bool ctrlKey, bool shiftKey, bool altKey);
	void onScroll(glm::vec2 scrollOffset, glm::vec2 mousePos, bool ctrlKey, bool shiftKey, bool altKey);

	Window* getWindow() { return &m_window; }

private:
	bool initWindowAndDevice();
	void terminateWindowAndDevice();

	bool initSwapChain();
	void terminateSwapChain();

	bool initDepthBuffer();
	void terminateDepthBuffer();

	bool initRenderPipeline();
	void terminateRenderPipeline();

	bool initTexture();
	void terminateTexture();

	bool initGeometry();
	void terminateGeometry();

	bool initUniforms();
	void terminateUniforms();

	bool initBindGroup();
	void terminateBindGroup();

	void updateProjectionMatrix();

private:
	// (Just aliases to make notations lighter)
	using mat4x4 = glm::mat4x4;
	using vec4 = glm::vec4;
	using vec3 = glm::vec3;
	using vec2 = glm::vec2;
	Window m_window;

	// Window and Device
	wgpu::Instance m_instance = nullptr;
	wgpu::Surface m_surface = nullptr;
	wgpu::Device m_device = nullptr;
	wgpu::Queue m_queue = nullptr;
	wgpu::TextureFormat m_swapChainFormat = wgpu::TextureFormat::Undefined;
	// Keep the error callback alive
	std::unique_ptr<wgpu::ErrorCallback> m_errorCallbackHandle;

	// Swap Chain
	wgpu::SwapChain m_swapChain = nullptr;

	// Depth Buffer
	wgpu::TextureFormat m_depthTextureFormat = wgpu::TextureFormat::Depth24Plus;
	wgpu::Texture m_depthTexture = nullptr;
	wgpu::TextureView m_depthTextureView = nullptr;

	// Render Pipeline
	wgpu::BindGroupLayout m_bindGroupLayout = nullptr;
	wgpu::ShaderModule m_shaderModule = nullptr;
	wgpu::RenderPipeline m_pipeline = nullptr;

	// Texture
	wgpu::Sampler m_sampler = nullptr;
	wgpu::Texture m_texture = nullptr;
	wgpu::TextureView m_textureView = nullptr;

	// Geometry
	wgpu::Buffer m_vertexBuffer = nullptr;
	int m_vertexCount = 0;


};
