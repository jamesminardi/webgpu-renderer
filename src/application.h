#pragma once

#include <numbers>
#include <webgpu/webgpu.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include "window.h"
#include "input.h"
#include "shader.h"
#include "camera.h"
#include "terrain.h"

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

/*
 * The same structure as in the shader, replicated in C++
 */
struct ShaderUniforms {
	// We add transform matrices
	glm::mat4x4 projectionMatrix;
	glm::mat4x4 viewMatrix;
	glm::mat4x4 modelMatrix;
	std::array<float, 4> color;
	float time;
	float _pad[3];
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
	void onMouseButton(Input::MouseButton button, Input::Action buttonAction, glm::vec2 mousePos, bool ctrlKey, bool shiftKey, bool altKey);
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


	void updateViewMatrix();

private:

	const std::string m_platformStr =
#if  defined(WEBGPU_BACKEND_DAWN)
			"Dawn";
#elif defined(WEBGPU_BACKEND_WGPU)
			"WGPU";
#else
			"Unknown";
#endif


	int m_size = 4;
	float m_scale = 1.0f;
	bool m_wireFrame = false;


	Terrain m_terrain;


	std::unique_ptr<Window> m_window;

	Camera m_camera;

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

	// Depth Buffer
	wgpu::TextureFormat m_depthTextureFormat = wgpu::TextureFormat::Depth24Plus;
	wgpu::Texture m_depthTexture = nullptr;
	wgpu::TextureView m_depthTextureView = nullptr;

	// Render Pipeline
	wgpu::ShaderModule m_shaderModule = nullptr;
	wgpu::BindGroupLayoutDescriptor m_bindGroupLayoutDesc{};
	wgpu::BindGroup m_bindGroup = nullptr;
	wgpu::BindGroupLayout m_bindGroupLayout = nullptr;
	wgpu::RenderPipeline m_pipeline = nullptr;

	// Texture
	wgpu::Sampler m_sampler = nullptr;
	wgpu::Texture m_texture = nullptr;
	wgpu::TextureView m_textureView = nullptr;

	wgpu::Buffer m_positionBuffer = nullptr;
	wgpu::Buffer m_colorBuffer = nullptr;
	wgpu::Buffer m_indexBuffer = nullptr;
	wgpu::Buffer m_uniformBuffer = nullptr;

	glm::vec2 m_mousePos = glm::vec2(0.0f);
	glm::vec2 m_mousePosNDC = glm::vec2(-1.0f, 1.0f);
	glm::vec3 m_mouseColor = glm::vec3(1.0f);

	float radius = 0.5f;
	float maxMouseRadius = radius + 0.02f;





	ShaderUniforms m_uniforms{};

	glm::vec3 focalPoint;

	// Rotate the object
	float angle1; // arbitrary time

	// Rotate the view point
	float angle2;

	float ratio;
	float focalLength;
	float near;
	float far;
	float divider;

	glm::mat4 S = glm::mat4(1.0);
	glm::mat4 T1 = glm::mat4(1.0);
	glm::mat4 R1 = glm::mat4(1.0);

	glm::mat4 R2 = glm::mat4(1.0);
	glm::mat4 T2 = glm::mat4(1.0);

	float fov;

};
