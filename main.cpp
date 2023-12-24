#include <GLFW/glfw3.h>
#include <webgpu/webgpu.h>
#include <iostream>

int main (int, char**) {
	// We create the equivalent of the navigator.gpu if this were web code

	// 1. We create a descriptor
	WGPUInstanceDescriptor desc = {};
	desc.nextInChain = nullptr;

	// 2. We create the instance using this descriptor
	WGPUInstance instance = wgpuCreateInstance(&desc);

	// 3. We can check whether there is actually an instance created
	if (!instance) {
		std::cerr << "Could not initialize WebGPU!" << std::endl;
		return 1;
	}

	// 4. Display the object (WGPUInstance is a simple pointer, it may be
	// copied around without worrying about its size).
	std::cout << "WGPU instance: " << instance << std::endl;

	if (!glfwInit()) {
		std::cerr << "Could not initialize GLFW!" << std::endl;
		return 1;
	}

	GLFWwindow* window = glfwCreateWindow(640, 480, "Learn WebGPU", NULL, NULL);
	if (!window) {
		std::cerr << "Could not open window!" << std::endl;
		glfwTerminate();
		return 1;
	}

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}

	// 5. We clean up the WebGPU instance
	wgpuInstanceRelease(instance);

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}