#pragma once

#include <numbers>
#include <webgpu/webgpu.hpp>
#include <glm/glm.hpp>
#include "window.h"
#include "input.h"
#include "shader.h"

/*
 * A structure that describes the data layout in the vertex buffer,
 * used by loadGeometryFromObj and used it in `sizeof` and `offsetof`
 * when uploading data to the GPU.
 */
struct VertexAttributes {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 uv;
};


class Application {
public:

	Application();
	~Application();

	void onFrame();

	bool isRunning();

	Window& getWindow() {
		return *m_window;
	}

	// Modifier parameters are true if they are pressed while onKey is called
	// TODO: Just have implementation call for modifier keys and mouse position using window get methods instead of passing in
	void onResize(int width, int height);
	void onKey([[maybe_unused]] [[maybe_unused]] Input::Key key, Input::Action buttonAction, bool ctrlKey, bool shiftKey, bool altKey);
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

	void initGui(); // called in onInit
	void terminateGui(); // called in onFinish
	void updateGui(wgpu::RenderPassEncoder renderPass); // called in onFrame


private:

	const std::string m_platformStr =
#if  defined(WEBGPU_BACKEND_DAWN)
			"Dawn";
#elif defined(WEBGPU_BACKEND_WGPU)
			"WGPU";
#else
			"Unknown";
#endif

	std::vector<uint16_t> m_indexData;

	// Vertex buffer
	// There are 2 floats per vertex, one for x and one for y.
	// But in the end this is just a bunch of floats to the eyes of the GPU,
	// the *layout* will tell how to interpret this.
	std::vector<float> m_positionData;

	// Color buffer, rgb
	std::vector<float> m_colorData;

	int numSides = 3;


	std::unique_ptr<Window> m_window;

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

	wgpu::Buffer m_positionBuffer = nullptr;
	wgpu::Buffer m_colorBuffer = nullptr;
	wgpu::Buffer m_indexBuffer = nullptr;
	int m_vertexCount = 0;
	int m_indexCount = 0;

	glm::vec2 m_mousePos = glm::vec2(0.0f);
	glm::vec2 m_mousePosNDC = glm::vec2(-1.0f, 1.0f);
	glm::vec3 m_mouseColor = glm::vec3(1.0f);

	float radius = 0.5f;
	float maxMouseRadius = radius + 0.02f;

};
