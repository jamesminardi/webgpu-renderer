#pragma once

#include "webgpu/webgpu.hpp"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
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

	static bool glfwInitialized;

	Window(WindowConfig* config, Application* app = nullptr);

	~Window();

	wgpu::Surface getSurface(wgpu::Instance instance);

	int shouldClose() const;

	void setTitle(const std::string& title);

	void enableApplicationCallbacks(Application* app);
	void disableApplicationCallbacks();

	glm::ivec2 getSize() const;
	int getWidth() const;
	int getHeight() const;
	float getAspectRatio() const;

	void inputSetCallbacks(Callbacks callbacks);
	glm::vec2 inputQueryMousePos() const;
	void inputPollEvents();


	// GLFW Event Handlers
	// -------------------
	static void glfwErrorCallback(int error, const char* description);
	static void glfwResizeCallback(GLFWwindow* window, int width, int height);
	static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void glfwMousePositionCallback(GLFWwindow* window, double x, double y);
	static void glfwMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
	static void glfwScrollCallback(GLFWwindow* window, double x, double y);



	float mouseScrollScaleFactor = 1.0f;

	GLFWwindow* handle;

private:

};


