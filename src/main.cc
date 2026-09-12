#include <print>
#include <string>
#include <cstdio>
#include <cstdlib>

#include <glad/glad.h> // Must be included before GLFW.
#include <GLFW/glfw3.h>

template <class ... Args>
void exit(std::format_string<Args ... > fmt, Args&& ... args) {
	std::print(stderr, fmt, std::forward<Args>(args) ... );
	std::exit(EXIT_FAILURE);
}

int main(int argc, char const* argv[]) {
	glfwSetErrorCallback([] (int code, char const* message) {
		std::print(stderr, "Error {}: {}\n", code, message);
	});

	if (!glfwInit()) exit("glfwInit: Unable to start GLFW.\n");
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#	if __APPLE__
 		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#	endif

	unsigned const W = 800;
	unsigned const H = 800;
	std::string const Title = "Hello, window!";
	GLFWwindow* window = glfwCreateWindow(W, H, Title.c_str(), {}, {});
	if (!window) exit("glfwCreateWindow: Unable to create window.\n");
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		exit("gladLoadGLLoader: Unable to start GLAD.\n");
	}

	glViewport(0, 0, W, H);

	glfwSetFramebufferSizeCallback(window, [] (GLFWwindow*, int w, int h) {
		glViewport(0, 0, w, h);
	});

	while (!glfwWindowShouldClose(window)) {
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return EXIT_SUCCESS;
}
