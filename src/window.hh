#include <functional>
#include <print>
#include <string>
#include <cstdio>
#include <cstdlib>

#include <glad/glad.h> // Must be included before GLFW.
#include <GLFW/glfw3.h>

// Programs right now can only have one monolithic window. When this window
// closes, everything closes.
class Window {
	GLFWwindow* m_handler {};

public:
	Window(
		unsigned startWidth, unsigned startHeight,
		std::string title,
		void (* renderLoop)(GLFWwindow*)
	) {
		// Make sure we log the most descriptive error messages available.
		glfwSetErrorCallback([] (int code, char const* message) {
			std::print(stderr, "Error {}: {}\n", code, message);
		});

		// Initialize with the correct OpenGL versioning.
		if (!glfwInit()) exit("glfwInit: Unable to start GLFW.\n");
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#		if __APPLE__
 			glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#		endif

		// Create window.
		m_handler = glfwCreateWindow(
			startWidth, startHeight,
			title.c_str(), {}, {}
		);
		if (!m_handler) exit("glfwCreateWindow: Unable to create window.\n");
		glfwMakeContextCurrent(m_handler);

		// GLAD is used to make calling GL functions easier.
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			exit("gladLoadGLLoader: Unable to start GLAD.\n");
		}

		// Set our width and height, and respond when a user resizes.
		glViewport(0, 0, startWidth, startHeight);
		glfwSetFramebufferSizeCallback(
			m_handler,
			[] (GLFWwindow* /**/, int newWidth, int newHeight) {
				glViewport(0, 0, newWidth, newHeight);
			}
		);

		// Immediately start looping after setup.
		while (!glfwWindowShouldClose(m_handler)) {
			renderLoop(m_handler);
		}
	}

	~Window() {
		glfwTerminate();
	}

private:
	template <class ... Args>
	void exit(std::format_string<Args ... > fmt, Args&& ... args) {
		std::print(stderr, fmt, std::forward<Args>(args) ... );
		std::exit(EXIT_FAILURE);
	}
};
