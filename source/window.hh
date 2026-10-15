#pragma once
#include "graphics.hh"
#include "util.hh"
#include <GLFW/glfw3.h>

// Programs right now can only have one monolithic window. When this window
// closes, everything closes.
template <class AppState=std::monostate>
class Window {
	GLFWwindow* handle_m {};
	std::array<unsigned, 2> size_m {};
	std::optional<std::array<unsigned, 2>> mouse_m;
	AppState appState_m {};
	Graphics graphics_m {};

public:
	using AppFn = void (Window<AppState>&, AppState&);

	Window(
		unsigned width, unsigned height,
		std::string title,
		AppFn* setup,
		AppFn* renderLoop,
		AppState appState={}
	) {
		size_m = {width, height};
		appState_m = appState;
		setupErrorLog();
		setupGlfw(title);
		graphics_m.load(glfwGetProcAddress, &size_m);
		setup(*this, appState_m);
		while (!glfwWindowShouldClose(handle_m)) {
			renderLoop(*this, appState_m);
			glfwSwapBuffers(handle_m);
			glfwPollEvents();
		}
	}

	~Window() {
		graphics_m.unload();
		glfwTerminate();
	}

	void close() {
		glfwSetWindowShouldClose(handle_m, true);
	}

	// Getters
	GLFWwindow* handle() const { return handle_m; }
	auto const& size() const { return size_m; }
	auto const& mouse() const { return mouse_m; }
	auto /* */& graphics() /* */ { return graphics_m; }
	auto const& graphics() const { return graphics_m; }

	// Setters
	void size(auto const& val) {
		glfwSetWindowSize(handle_m, std::get<0>(val), std::get<1>(val));
	}

	// GLFW specific
	template <class ... Args> using CallbackFn = void (GLFWwindow*, Args ... );
	template <class Fn> using CallbackSetterFn = Fn* (GLFWwindow*, Fn*);

	template <class ... Args>
	void bind(
		CallbackSetterFn<CallbackFn<Args ... >>& glfwCallbackSetter,
		auto const& userFunction
	) {
		// WARNING: Assumes all GLFW callback setters have a unique type.
		static auto uf = userFunction;
		glfwCallbackSetter(handle_m,
			[] (GLFWwindow*, Args ... args) {
				uf(args ... );
			}
		);
	}

private:
	void setupErrorLog() {
		glfwSetErrorCallback([] (int code, char const* message) {
			std::print(stderr, "GLFW error {}: {}\n", code, message);
		});
	}

	void setupGlfw(std::string const& title) {
		// Initialize with the correct OpenGL versioning.
		if (!glfwInit()) exit("glfwInit: Unable to start GLFW.\n");
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#		if __APPLE__
 			glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#		endif

		// Create our window, keeping track of its handle.
		handle_m = glfwCreateWindow(
			size_m[0], size_m[1],
			title.c_str(),
			{}, // no monitor
			{} // no resource sharing
		);
		if (!handle_m) exit("glfwCreateWindow: Unable to create window.\n");
		glfwMakeContextCurrent(handle_m);

		// Set our width and height, and respond when a user resizes.
		bind(glfwSetFramebufferSizeCallback,
			[&] (unsigned newWidth, unsigned newHeight) {
				size_m = {newWidth, newHeight};
			}
		);

		// Keep track of mouse input, when it's available.
		bind(glfwSetCursorPosCallback,
			[&] (unsigned mouseX, unsigned mouseY) {
				if (!mouse_m) mouse_m = {0, 0};
				(*mouse_m)[0] = mouseX;
				(*mouse_m)[1] = size_m[1] - mouseY;
			}
		);

		// Otherwise report no position.
		bind(glfwSetCursorEnterCallback,
			[&] (bool entered) {
				if (!entered) mouse_m = std::nullopt;
			}
		);
	}
};
