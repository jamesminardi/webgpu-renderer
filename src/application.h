#pragma once

#include <numbers>
#include <webgpu/webgpu.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>
#include "input.h"
#include "shader.h"
#include "camera.h"
#include "noise/noise.h"
#include "types.h"

#include <string>


class World;
//class TerrainRenderer;


class Application {
public:

	static std::unique_ptr<wgpu::Device> device;
	static std::unique_ptr<wgpu::Queue> queue;

	static wgpu::TextureFormat swapChainFormat;
	static wgpu::TextureFormat depthTextureFormat;

//	TerrainRenderer terrainRenderer;


	Application();
	~Application();

	void onFrame();

	bool isRunning();

//    wgpu::Device& getDevice() {
//        return m_device;
//    }
//
//    wgpu::Queue& getQueue() {
//        return m_queue;
//    }

    wgpu::SwapChain& getSwapChain() {
        return m_swapChain;
    }



	// Modifier parameters are true if they are pressed while onKey is called
	// TODO: Just have implementation call for modifier keys and mouse position using window get methods instead of passing in?
	void onResize(int width, int height);
	void onKey([[maybe_unused]] [[maybe_unused]] Input::Key key, Input::Action buttonAction, bool ctrlKey, bool shiftKey, bool altKey);
	void onMouseMove(glm::vec2 mousePos, bool ctrlKey, bool shiftKey, bool altKey);
	void onMouseButton(Input::MouseButton button, Input::Action buttonAction, glm::vec2 mousePos, bool ctrlKey, bool shiftKey, bool altKey);
	void onScroll(glm::vec2 scrollOffset, glm::vec2 mousePos, bool ctrlKey, bool shiftKey, bool altKey);

private:

	void initWorld();
	void terminateWorld();

	void initWindowAndDevice();
	void terminateWindowAndDevice();

	void initSwapChain();
	void terminateSwapChain();

	void initDepthBuffer();
	void terminateDepthBuffer();

//	void initRenderPipeline();
//	void terminateRenderPipeline();

	void initTexture();
	void terminateTexture();

//	void initGeometry();
//	void terminateGeometry();
//
//	void initUniforms();
//	void terminateUniforms();
//
//	void initBindGroup();
//	void terminateBindGroup();

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

	Noise::Descriptor noiseDesc{};

//	Noise noise;
//	Chunk chunk;

	// updated
	std::unique_ptr<World> world;



//	Camera m_camera;

	// Window and Device
	wgpu::Instance m_instance = nullptr;
	wgpu::Surface m_surface = nullptr;
//	wgpu::Device m_device = nullptr;
//	wgpu::Queue m_queue = nullptr;

	// Keep the error callback alive
	std::unique_ptr<wgpu::ErrorCallback> m_errorCallbackHandle;
	std::unique_ptr<wgpu::QueueWorkDoneCallback> m_queueWorkDoneCallbackHandle;

#ifdef WEBGPU_BACKEND_DAWN
	std::unique_ptr<wgpu::DeviceLostCallback>  m_deviceLostCallbackHandle;
#endif

	// Swap Chain
	wgpu::SwapChain m_swapChain = nullptr;

	// Depth Buffer
	wgpu::Texture m_depthTexture = nullptr;
	wgpu::TextureView m_depthTextureView = nullptr;

	// Render Pipeline
//	wgpu::ShaderModule m_shaderModule = nullptr;
//	wgpu::BindGroupLayoutDescriptor m_bindGroupLayoutDesc{};
//	wgpu::BindGroup m_bindGroup = nullptr;
//	wgpu::BindGroupLayout m_bindGroupLayout = nullptr;
//	wgpu::RenderPipeline m_pipeline = nullptr;

	// Texture
	wgpu::Sampler m_sampler = nullptr;
	wgpu::Texture m_texture = nullptr;
	wgpu::TextureView m_textureView = nullptr;

//    wgpu::Buffer m_vertexBuffer = nullptr;
//	wgpu::Buffer m_indexBuffer = nullptr;
//	wgpu::Buffer m_uniformBuffer = nullptr;

	glm::vec2 m_mousePos = glm::vec2(0.0f);
	glm::vec2 m_mousePosNDC = glm::vec2(-1.0f, 1.0f);
	glm::vec3 m_mouseColor = glm::vec3(1.0f);

	float radius = 0.5f;
	float maxMouseRadius = radius + 0.02f;



	ShaderUniforms m_uniforms{};


public:

	glm::vec3 focalPoint{};

	float ratio{};
	float focalLength{};
	float near{};
	float far{};
	float divider{};

	glm::mat4 S = glm::mat4(1.0);
	glm::mat4 T1 = glm::mat4(1.0);
	glm::mat4 R1 = glm::mat4(1.0);

	glm::mat4 R2 = glm::mat4(1.0);
	glm::mat4 T2 = glm::mat4(1.0);

	float fov{};

};
