#pragma once

#include <webgpu/webgpu.hpp>
#include <glm/glm.hpp>
#include "window.h"
#include "input.h"

class Application {
public:

	Application();
	~Application();

	void onFrame();

	bool isRunning();

	Window* getWindow() {
		return m_window;
	}

	// Modifier parameters are true if they are pressed while onKey is called
	// TODO: Just have implementation call for modifier keys and mouse position using window get methods instead of passing in
	void onResize(int width, int height);
	void onKey(Input::Key key, Input::Action buttonAction, bool ctrlKey, bool shiftKey, bool altKey);
	void onMouseMove(glm::vec2 mousePos, bool ctrlKey, bool shiftKey, bool altKey);
	void onMouseClick(Input::MouseButton button, Input::Action buttonAction, glm::vec2 mousePos, bool ctrlKey, bool shiftKey, bool altKey);
	void onScroll(glm::vec2 scrollOffset, glm::vec2 mousePos, bool ctrlKey, bool shiftKey, bool altKey);

private:
	void initWindowAndDevice();
	void terminateWindowAndDevice();

	void initSwapChain();
	void terminateSwapChain();

	void initDepthBuffer();
	void terminateDepthBuffer();

	void initRenderPipeline();
	void terminateRenderPipeline();

	void initTexture();
	void terminateTexture();

	void initGeometry();
	void terminateGeometry();

	void initUniforms();
	void terminateUniforms();

	void initBindGroup();
	void terminateBindGroup();

private:

	Window* m_window;

	// Window and Device
	wgpu::Instance m_instance = nullptr;
	wgpu::Surface m_surface = nullptr;
	wgpu::Device m_device = nullptr;
	wgpu::Queue m_queue = nullptr;
	wgpu::TextureFormat m_swapChainFormat = wgpu::TextureFormat::Undefined;

	// Keep the error callback alive
	std::unique_ptr<wgpu::ErrorCallback> m_errorCallbackHandle;
	std::unique_ptr<wgpu::QueueWorkDoneCallback> m_queueWorkDoneCallbackHandle;

#ifdef WEBGPU_BACKEND_DAWN
	std::unique_ptr<wgpu::DeviceLostCallback>  m_deviceLostCallbackHandle;
#endif

	// Swap Chain
	wgpu::SwapChain m_swapChain = nullptr;

	// Render Pipeline
	wgpu::ShaderModule m_shaderModule = nullptr;
	wgpu::RenderPipeline m_pipeline = nullptr;

};
