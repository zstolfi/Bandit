#pragma once
#include "util.hh"
#include <glad/glad.h> // Must be included before GLFW.
#include <GLFW/glfw3.h>

// Programs right now can only have one monolithic window. When this window
// closes, everything closes.
class Window {
	GLFWwindow* m_handler {};

public:
	Window(
		unsigned width, unsigned height, std::string title,
		void (* renderLoop)(Window&)
	) {
		setupErrorLog();
		setupWindow(width, height, title);
		setupScene();
		while (!glfwWindowShouldClose(m_handler)) {
			renderLoop(*this);
		}
	}

	~Window() {
		glfwTerminate();
	}

	// Getters
	GLFWwindow* handler() const { return m_handler; }

private:
	void setupErrorLog() {
		glfwSetErrorCallback([] (int code, char const* message) {
			std::print(stderr, "GLFW error {}: {}\n", code, message);
		});
	}

	void setupWindow(unsigned width, unsigned height, std::string title) {
		// Initialize with the correct OpenGL versioning.
		if (!glfwInit()) exit("glfwInit: Unable to start GLFW.\n");
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#		if __APPLE__
 			glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#		endif

		// Create window.
		m_handler = glfwCreateWindow(width, height, title.c_str(), {}, {});
		if (!m_handler) exit("glfwCreateWindow: Unable to create window.\n");
		glfwMakeContextCurrent(m_handler);

		// GLAD is used to make calling GL functions easier.
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			exit("gladLoadGLLoader: Unable to start GLAD.\n");
		}

		// Set our width and height, and respond when a user resizes.
		glViewport(0, 0, width, height);
		glfwSetFramebufferSizeCallback(
			m_handler,
			[] (GLFWwindow* /**/, int newWidth, int newHeight) {
				glViewport(0, 0, newWidth, newHeight);
			}
		);
	}

	void setupScene() {

	}
};
