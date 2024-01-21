#pragma once

#include "webgpu/webgpu.hpp"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"

#include <imgui.h>
#include <backends/imgui_impl_wgpu.h>
#include <backends/imgui_impl_glfw.h>

#include <string>


struct WindowConfig {
	std::string title = "";
	int width = 640;
	int height = 480;
	bool resizable = false;
};

// Callbacks for user to implement. These are called by the window class.
struct Callbacks {
	void (*onResize)(GLFWwindow* window, int width, int height);
//	void (*onKey)(GLFWwindow* window, int key, int scancode, int action, int mods);
//	void (*onMouse)(GLFWwindow* window, int button, int action, int mods);
//	void (*onMouseMove)(GLFWwindow* window, double x, double y);
//	void (*onScroll)(GLFWwindow* window, double x, double y);
};

/*
 * Window class that wraps GLFW.
 *
 * Input callbacks are also wrapped, and can be set using the Callbacks struct.
 */

class Application;

class Window {
public:

	Window(WindowConfig* config, Application* app = nullptr);

	~Window();

	static bool glfwInitialized;
	static void inputPollEvents();

	wgpu::Surface getSurface(wgpu::Instance instance) const;

	int shouldClose() const;

	void setTitle(const std::string& title);

	[[maybe_unused]] void enableApplicationCallbacks(Application* app);
	void disableApplicationCallbacks();

	glm::ivec2 getSize() const;
	int getWidth() const;
	int getHeight() const;

	float getAspectRatio() const;

	glm::vec2 getMousePos() const;


	//

	// Convert window coordinates to normalized device coordinates
	glm::vec2 windowCoordsToNDC(const glm::vec2 windowCoords) const;

	// GLFW Event Handlers
	// -------------------
	static void glfwErrorCallback(int error, const char* description);
	static void glfwResizeCallback(GLFWwindow* window, int width, int height);
	static void glfwKeyCallback(GLFWwindow* window, int key, [[maybe_unused]] int scancode, int action, int mods);
	static void glfwMousePositionCallback(GLFWwindow* window, double x, double y);
	static void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void glfwScrollCallback(GLFWwindow* window, double x, double y);



	float mouseScrollScaleFactor = 1.0f;

	GLFWwindow* handle;

private:

};


