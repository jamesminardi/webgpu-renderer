
#include <glfw3webgpu.h>
#include "application.h"
#include "window.h"
#include "input.h"

bool Window::glfwInitialized = false;

Window::Window(WindowConfig* config, Application* app) {
	if(!config) {
	return;
	}


	// TODO: Move to a static init function or application class init
	if (!glfwInitialized) {
		if (!glfwInit()) {
			throw std::runtime_error("Could not initialize GLFW!");
		}
		glfwInitialized = true;
	}

	glfwWindowHint(GLFW_RESIZABLE, config->resizable); // Not dealing with resizing for now
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); // Tell GLFW to not set up any API specific stuff

	handle = glfwCreateWindow(config->width, config->height, config->title.c_str(), nullptr, nullptr);
	if (!handle) {
		glfwTerminate();
		throw std::runtime_error("Could not open window!");
	}

	glfwSetWindowUserPointer(handle, app);

	glfwSetWindowSizeCallback(handle, glfwResizeCallback);
	glfwSetErrorCallback(glfwErrorCallback);
	glfwSetKeyCallback(handle, glfwKeyCallback);
	glfwSetCursorPosCallback(handle, glfwMousePositionCallback);
	glfwSetMouseButtonCallback(handle, glfwMouseButtonCallback);
	glfwSetScrollCallback(handle, glfwScrollCallback);


}

Window::~Window() {
	if (this->handle) {
		glfwDestroyWindow(handle);
	}
	glfwTerminate();
}

wgpu::Surface Window::getSurface(wgpu::Instance instance) {
	return glfwGetWGPUSurface(instance, handle);
}

int Window::shouldClose() const {
	return glfwWindowShouldClose(handle);
}

void Window::setTitle(const std::string& title) {
	glfwSetWindowTitle(handle, title.c_str());
}

void Window::enableApplicationCallbacks(Application* app) {
	glfwSetWindowUserPointer(handle, (void *)app);
}

void Window::disableApplicationCallbacks() {
	glfwSetWindowUserPointer(handle, (void *)nullptr);
}

glm::ivec2 Window::getSize() const {
	int width, height;
	glfwGetWindowSize(handle, &width, &height);
	return {width, height};
}

int Window::getWidth() const {
	return getSize().x;
}

int Window::getHeight() const {
	return getSize().y;
}

float Window::getAspectRatio() const {
	auto size = getSize();
	return (float)size.x / (float)size.y;
}

glm::vec2 Window::inputQueryMousePos() const {
	double x, y;
	glfwGetCursorPos(handle, &x, &y);
	return {(float)x, (float)y};
}

void inputPollEvents() {
	glfwPollEvents();
}


// GLFW Event Handlers
// -------------------

/*
 * Room for abstraction with multipe sets of inputs: A separate input class for UI, in-game, etc.
 * Check out: https://stackoverflow.com/questions/55573238/how-do-i-do-a-proper-input-class-in-glfw-for-a-game-engine
 * And add controller support by implementing an input interface
 */

static Input::Key remapGlfwKeyCode(int key);

void Window::glfwErrorCallback(int error, const char* description) {
	std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

void Window::glfwResizeCallback(GLFWwindow* window, int width, int height) {
	auto* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	if (app) {
		app->onResize(width, height);
	}
}

void Window::glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
		return;
	}

	auto* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	if (app) {

		const bool ctrlKey = bool(mods & GLFW_MOD_CONTROL);
		const bool shiftKey = bool(mods & GLFW_MOD_SHIFT);
		const bool altKey = bool(mods & GLFW_MOD_ALT);

		Input::Key keyCode = remapGlfwKeyCode(key);

		auto buttonAction = Input::Action::Undefined;
		if (action == GLFW_PRESS) {
			buttonAction = Input::Action::Press;
		} else if (action == GLFW_RELEASE) {
			buttonAction = Input::Action::Release;
		} else if (action == GLFW_REPEAT) {
			buttonAction = Input::Action::Repeat;
		}

		app->onKey(keyCode, buttonAction, ctrlKey, shiftKey, altKey);
	}
}

void Window::glfwMousePositionCallback(GLFWwindow* window, double x, double y) {
	auto* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	if (app) {

		bool ctrlKey = bool(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);
		bool shiftKey = bool(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
		bool altKey = bool(glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS);
		app->onMouseMove({(float)x, (float)y}, ctrlKey, shiftKey, altKey);
	}
}

void Window::glfwMouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
	auto* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	if (app) {

		auto buttonType = Input::MouseButton::Undefined;
		switch (button) {
			case GLFW_MOUSE_BUTTON_LEFT:
				buttonType = Input::MouseButton::Left;
				break;
			case GLFW_MOUSE_BUTTON_RIGHT:
				buttonType = Input::MouseButton::Right;
				break;
			case GLFW_MOUSE_BUTTON_MIDDLE:
				buttonType = Input::MouseButton::Middle;
				break;
			default:
				break;
		}

		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);

		bool ctrlKey = bool(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);
		bool shiftKey = bool(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
		bool altKey = bool(glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS);

		auto buttonAction = Input::Action::Undefined;
		if (action == GLFW_PRESS) {
			buttonAction = Input::Action::Press;
		} else if (action == GLFW_RELEASE) {
			buttonAction = Input::Action::Release;
		} else if (action == GLFW_REPEAT) {
			buttonAction = Input::Action::Repeat;
		}
		app->onMouseClick(buttonType, buttonAction, {(float)xPos, (float)yPos}, ctrlKey, shiftKey, altKey);
	}
}

float rescaleMouseScroll(double offset, float scaleFactor) {
	return (float)offset * scaleFactor;
}

void Window::glfwScrollCallback(GLFWwindow *window, double xOffset, double yOffset) {
	auto* app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
	if (app) {
		double xPos, yPos;
		glfwGetCursorPos(window, &xPos, &yPos);

		bool ctrlKey = bool(glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS);
		bool shiftKey = bool(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);
		bool altKey = bool(glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS);
		app->onScroll({rescaleMouseScroll(xOffset, app->getWindow()->mouseScrollScaleFactor),
					   rescaleMouseScroll(yOffset, app->getWindow()->mouseScrollScaleFactor)},
					  {(float)xPos, (float)yPos},  ctrlKey, shiftKey, altKey);
	}
}


static Input::Key remapGlfwKeyCode(int key)
{
	Input::Key key_code = Input::Key::Unknown;
	switch (key) {
		case GLFW_KEY_ESCAPE:
			key_code = Input::Key::Escape;
			break;
		case GLFW_KEY_TAB:
			key_code = Input::Key::Tab;
			break;
		case GLFW_KEY_LEFT_SHIFT:
			key_code = Input::Key::LeftShift;
			break;
		case GLFW_KEY_RIGHT_SHIFT:
			key_code = Input::Key::RightShift;
			break;
		case GLFW_KEY_LEFT_CONTROL:
			key_code = Input::Key::LeftControl;
			break;
		case GLFW_KEY_RIGHT_CONTROL:
			key_code = Input::Key::RightControl;
			break;
		case GLFW_KEY_LEFT_ALT:
			key_code = Input::Key::LeftAlt;
			break;
		case GLFW_KEY_RIGHT_ALT:
			key_code = Input::Key::RightAlt;
			break;
		case GLFW_KEY_LEFT_SUPER:
			key_code = Input::Key::LeftSuper;
			break;
		case GLFW_KEY_RIGHT_SUPER:
			key_code = Input::Key::RightSuper;
			break;
		case GLFW_KEY_MENU:
			key_code = Input::Key::Menu;
			break;
		case GLFW_KEY_NUM_LOCK:
			key_code = Input::Key::NumLock;
			break;
		case GLFW_KEY_CAPS_LOCK:
			key_code = Input::Key::CapsLock;
			break;
		case GLFW_KEY_PRINT_SCREEN:
			key_code = Input::Key::PrintScreen;
			break;
		case GLFW_KEY_SCROLL_LOCK:
			key_code = Input::Key::ScrollLock;
			break;
		case GLFW_KEY_PAUSE:
			key_code = Input::Key::Pause;
			break;
		case GLFW_KEY_DELETE:
			key_code = Input::Key::Delete;
			break;
		case GLFW_KEY_BACKSPACE:
			key_code = Input::Key::Backspace;
			break;
		case GLFW_KEY_ENTER:
			key_code = Input::Key::Enter;
			break;
		case GLFW_KEY_HOME:
			key_code = Input::Key::Home;
			break;
		case GLFW_KEY_END:
			key_code = Input::Key::End;
			break;
		case GLFW_KEY_PAGE_UP:
			key_code = Input::Key::PageUp;
			break;
		case GLFW_KEY_PAGE_DOWN:
			key_code = Input::Key::PageDown;
			break;
		case GLFW_KEY_INSERT:
			key_code = Input::Key::Insert;
			break;
		case GLFW_KEY_LEFT:
			key_code = Input::Key::Left;
			break;
		case GLFW_KEY_RIGHT:
			key_code = Input::Key::Right;
			break;
		case GLFW_KEY_DOWN:
			key_code = Input::Key::Down;
			break;
		case GLFW_KEY_UP:
			key_code = Input::Key::Up;
			break;
		case GLFW_KEY_F1:
			key_code = Input::Key::F1;
			break;
		case GLFW_KEY_F2:
			key_code = Input::Key::F2;
			break;
		case GLFW_KEY_F3:
			key_code = Input::Key::F3;
			break;
		case GLFW_KEY_F4:
			key_code = Input::Key::F4;
			break;
		case GLFW_KEY_F5:
			key_code = Input::Key::F5;
			break;
		case GLFW_KEY_F6:
			key_code = Input::Key::F6;
			break;
		case GLFW_KEY_F7:
			key_code = Input::Key::F7;
			break;
		case GLFW_KEY_F8:
			key_code = Input::Key::F8;
			break;
		case GLFW_KEY_F9:
			key_code = Input::Key::F9;
			break;
		case GLFW_KEY_F10:
			key_code = Input::Key::F10;
			break;
		case GLFW_KEY_F11:
			key_code = Input::Key::F11;
			break;
		case GLFW_KEY_F12:
			key_code = Input::Key::F12;
			break;
		case GLFW_KEY_F13:
			key_code = Input::Key::F13;
			break;
		case GLFW_KEY_F14:
			key_code = Input::Key::F14;
			break;
		case GLFW_KEY_F15:
			key_code = Input::Key::F15;
			break;
		case GLFW_KEY_F16:
			key_code = Input::Key::F16;
			break;
		case GLFW_KEY_F17:
			key_code = Input::Key::F17;
			break;
		case GLFW_KEY_F18:
			key_code = Input::Key::F18;
			break;
		case GLFW_KEY_F19:
			key_code = Input::Key::F19;
			break;
		case GLFW_KEY_F20:
			key_code = Input::Key::F20;
			break;
		case GLFW_KEY_F21:
			key_code = Input::Key::F21;
			break;
		case GLFW_KEY_F22:
			key_code = Input::Key::F22;
			break;
		case GLFW_KEY_F23:
			key_code = Input::Key::F23;
			break;
		case GLFW_KEY_F24:
			key_code = Input::Key::F24;
			break;
		case GLFW_KEY_F25:
			key_code = Input::Key::F25;
			break;

			/* Numeric keypad */
		case GLFW_KEY_KP_DIVIDE:
			key_code = Input::Key::KpDivide;
			break;
		case GLFW_KEY_KP_MULTIPLY:
			key_code = Input::Key::KpMultiply;
			break;
		case GLFW_KEY_KP_SUBTRACT:
			key_code = Input::Key::KpSubtract;
			break;
		case GLFW_KEY_KP_ADD:
			key_code = Input::Key::KpAdd;
			break;

			/* These should have been detected in secondary keysym test above! */
		case GLFW_KEY_KP_0:
			key_code = Input::Key::Kp0;
			break;
		case GLFW_KEY_KP_1:
			key_code = Input::Key::Kp1;
			break;
		case GLFW_KEY_KP_2:
			key_code = Input::Key::Kp2;
			break;
		case GLFW_KEY_KP_3:
			key_code = Input::Key::Kp3;
			break;
		case GLFW_KEY_KP_4:
			key_code = Input::Key::Kp4;
			break;
		case GLFW_KEY_KP_5:
			key_code = Input::Key::Kp5;
			break;
		case GLFW_KEY_KP_6:
			key_code = Input::Key::Kp6;
			break;
		case GLFW_KEY_KP_7:
			key_code = Input::Key::Kp7;
			break;
		case GLFW_KEY_KP_8:
			key_code = Input::Key::Kp8;
			break;
		case GLFW_KEY_KP_9:
			key_code = Input::Key::Kp9;
			break;
		case GLFW_KEY_KP_DECIMAL:
			key_code = Input::Key::KpDecimal;
			break;
		case GLFW_KEY_KP_EQUAL:
			key_code = Input::Key::KpEqual;
			break;
		case GLFW_KEY_KP_ENTER:
			key_code = Input::Key::KpEnter;
			break;

			/*
			 * Last resort: Check for printable keys (should not happen if the XKB
			 * extension is available). This will give a layout dependent mapping
			 * (which is wrong, and we may miss some keys, especially on non-US
			 * keyboards), but it's better than nothing...
			 */
		case GLFW_KEY_A:
			key_code = Input::Key::A;
			break;
		case GLFW_KEY_B:
			key_code = Input::Key::B;
			break;
		case GLFW_KEY_C:
			key_code = Input::Key::C;
			break;
		case GLFW_KEY_D:
			key_code = Input::Key::D;
			break;
		case GLFW_KEY_E:
			key_code = Input::Key::E;
			break;
		case GLFW_KEY_F:
			key_code = Input::Key::F;
			break;
		case GLFW_KEY_G:
			key_code = Input::Key::G;
			break;
		case GLFW_KEY_H:
			key_code = Input::Key::H;
			break;
		case GLFW_KEY_I:
			key_code = Input::Key::I;
			break;
		case GLFW_KEY_J:
			key_code = Input::Key::J;
			break;
		case GLFW_KEY_K:
			key_code = Input::Key::K;
			break;
		case GLFW_KEY_L:
			key_code = Input::Key::L;
			break;
		case GLFW_KEY_M:
			key_code = Input::Key::M;
			break;
		case GLFW_KEY_N:
			key_code = Input::Key::N;
			break;
		case GLFW_KEY_O:
			key_code = Input::Key::O;
			break;
		case GLFW_KEY_P:
			key_code = Input::Key::P;
			break;
		case GLFW_KEY_Q:
			key_code = Input::Key::Q;
			break;
		case GLFW_KEY_R:
			key_code = Input::Key::R;
			break;
		case GLFW_KEY_S:
			key_code = Input::Key::S;
			break;
		case GLFW_KEY_T:
			key_code = Input::Key::T;
			break;
		case GLFW_KEY_U:
			key_code = Input::Key::U;
			break;
		case GLFW_KEY_V:
			key_code = Input::Key::V;
			break;
		case GLFW_KEY_W:
			key_code = Input::Key::W;
			break;
		case GLFW_KEY_X:
			key_code = Input::Key::X;
			break;
		case GLFW_KEY_Y:
			key_code = Input::Key::Y;
			break;
		case GLFW_KEY_Z:
			key_code = Input::Key::Z;
			break;
		case GLFW_KEY_1:
			key_code = Input::Key::One;
			break;
		case GLFW_KEY_2:
			key_code = Input::Key::Two;
			break;
		case GLFW_KEY_3:
			key_code = Input::Key::Three;
			break;
		case GLFW_KEY_4:
			key_code = Input::Key::Four;
			break;
		case GLFW_KEY_5:
			key_code = Input::Key::Five;
			break;
		case GLFW_KEY_6:
			key_code = Input::Key::Six;
			break;
		case GLFW_KEY_7:
			key_code = Input::Key::Seven;
			break;
		case GLFW_KEY_8:
			key_code = Input::Key::Eight;
			break;
		case GLFW_KEY_9:
			key_code = Input::Key::Nine;
			break;
		case GLFW_KEY_0:
			key_code = Input::Key::Zero;
			break;
		case GLFW_KEY_SPACE:
			key_code = Input::Key::Space;
			break;
		case GLFW_KEY_MINUS:
			key_code = Input::Key::Minus;
			break;
		case GLFW_KEY_EQUAL:
			key_code = Input::Key::Equal;
			break;
		case GLFW_KEY_LEFT_BRACKET:
			key_code = Input::Key::LeftBracket;
			break;
		case GLFW_KEY_RIGHT_BRACKET:
			key_code = Input::Key::RightBracket;
			break;
		case GLFW_KEY_BACKSLASH:
			key_code = Input::Key::Backslash;
			break;
		case GLFW_KEY_SEMICOLON:
			key_code = Input::Key::Semicolon;
			break;
		case GLFW_KEY_APOSTROPHE:
			key_code = Input::Key::Apostrophe;
			break;
		case GLFW_KEY_GRAVE_ACCENT:
			key_code = Input::Key::GraveAccent;
			break;
		case GLFW_KEY_COMMA:
			key_code = Input::Key::Comma;
			break;
		case GLFW_KEY_PERIOD:
			key_code = Input::Key::Period;
			break;
		case GLFW_KEY_SLASH:
			key_code = Input::Key::Slash;
			break;
		case GLFW_KEY_WORLD_1:
			key_code = Input::Key::World1;  /* At least in some layouts... */
			break;
		case GLFW_KEY_WORLD_2:
			key_code = Input::Key::World2;  /* At least in some layouts... */
			break;
		default:
			key_code = Input::Key::Unknown; /* No matching translation was found */
			break;
	}

	return key_code;
}
