#include "window.hh"
#include "util.hh"
#include <array>
#include <string_view>

void renderLoop(Window& window) {
	auto* handler = window.handler();
	if (glfwGetKey(handler, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(handler, true);
	}

	glClearColor(0.9, 0.9, 0.9, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(window.shaderProgram());
	glBindVertexArray(window.VAO());
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glfwSwapBuffers(handler);
	glfwPollEvents();
}

int main(int argc, char const* argv[]) {
	Window {
		800, 600,
		"Hello, triangle!",
		renderLoop
	};

	return EXIT_SUCCESS;
}
