#include "application.h"


Application::Application() : m_window(nullptr) {
	m_instance = createInstance(wgpu::InstanceDescriptor{});
	if (!m_instance) {
		throw std::runtime_error("Could not initialize WebGPU!");
	}

	WindowConfig windowConfig{};
	windowConfig.title = "Learn WebGPU";
	windowConfig.width = 640;
	windowConfig.height = 480;
	windowConfig.resizable = false;
	m_window = Window(&windowConfig);


}

void Application::onResize(int width, int height) {

}

void Application::onKey(Input::Key key, Input::Action buttonAction, bool ctrlKey, bool shiftKey, bool altKey) {

}

void Application::onMouseMove(glm::vec2 mousePos, bool ctrlKey, bool shiftKey, bool altKey) {

}

void Application::onMouseClick(Input::MouseButton button, Input::Action buttonAction, glm::vec2 mousePos, bool ctrlKey, bool shiftKey, bool altKey) {

}

void Application::onScroll(glm::vec2 scrollOffset, glm::vec2 mousePos, bool ctrlKey, bool shiftKey, bool altKey) {

}



