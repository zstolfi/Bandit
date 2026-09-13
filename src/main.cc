#include "window.hh"

void renderLoop(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	glClearColor(0.9, 0.9, 0.9, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glfwSwapBuffers(window);
	glfwPollEvents();
}

int main(int argc, char const* argv[]) {
	Window {
		800, 600,
		"Hello, window class!",
		renderLoop
	};

	return EXIT_SUCCESS;
}
