#include "window.hh"
#include "util.hh"

struct Coord {
	float x {}, y {};
};

struct Rectangle {
	float x0 {}, y0 {};
	float x1 {}, y1 {};
};

// https://www.desmos.com/calculator/6pqhh5uipy
auto squarePositions(float pieceSize) {
	auto result = std::vector<std::array<Coord, 4>> {};
	float const s = 0.5, c = 0.5 * std::sqrt(3);
	float const radius = 0.05 * pieceSize;

	auto square = [&] (Coord v, int angle) {
		auto result = std::array<Coord, 4> {};
		/**/ if (angle == 0) result = {{{1, 1}, {-1, 1}, {-1, -1}, {1, -1}}};
		else if (angle == 1) result = {{{s, c}, {-c, s}, {-s, -c}, {c, -s}}};
		else if (angle == 2) result = {{{c, s}, {-s, c}, {-c, -s}, {s, -c}}};
		for (Coord& c : result) c.x *= radius, c.y *= radius;
		for (Coord& c : result) c.x += v.x   , c.y += v.y;
		return result;
	};

	auto corners = std::array<Coord, 8> {{
		{ -0.7,  0.2 },
		{ -0.7, -0.2 },
		{ -0.3,  0.2 },
		{ -0.3, -0.2 },
		{  0.3,  0.2 },
		{  0.3, -0.2 },
		{  0.7,  0.2 },
		{  0.7, -0.2 },
	}};

	result = {
		square({corners[0].x + (   1)*radius, corners[0].y + (  -1)*radius}, 0),
		square({corners[1].x + (   1)*radius, corners[1].y + (   1)*radius}, 0),
		square({corners[2].x + (  -1)*radius, corners[2].y + (  -1)*radius}, 0),
		square({corners[3].x + (  -1)*radius, corners[3].y + (   1)*radius}, 0),

		square({corners[4].x + (   1)*radius, corners[4].y + (  -1)*radius}, 0),
		square({corners[5].x + (   1)*radius, corners[5].y + (   1)*radius}, 0),
		square({corners[6].x + (  -1)*radius, corners[6].y + (  -1)*radius}, 0),
		square({corners[7].x + (  -1)*radius, corners[7].y + (   1)*radius}, 0),

		square({corners[0].x + ( c-s)*radius, corners[0].y + ( c+s)*radius}, 1),
		square({corners[2].x + (-c+s)*radius, corners[2].y + ( c+s)*radius}, 2),
		square({corners[4].x + ( c-s)*radius, corners[4].y + ( c+s)*radius}, 1),
		square({corners[6].x + (-c+s)*radius, corners[6].y + ( c+s)*radius}, 2),

		square({corners[1].x + ( c-s)*radius, corners[1].y + (-c-s)*radius}, 2),
		square({corners[3].x + (-c+s)*radius, corners[3].y + (-c-s)*radius}, 1),
		square({corners[5].x + ( c-s)*radius, corners[5].y + (-c-s)*radius}, 2),
		square({corners[7].x + (-c+s)*radius, corners[7].y + (-c-s)*radius}, 1),

		square({corners[0].x + (-c-s)*radius, corners[0].y + (-c+s)*radius}, 2),
		square({corners[1].x + (-c-s)*radius, corners[1].y + ( c-s)*radius}, 1),
		square({corners[6].x + ( c+s)*radius, corners[6].y + (-c+s)*radius}, 1),
		square({corners[7].x + ( c+s)*radius, corners[7].y + ( c-s)*radius}, 2),

		square({corners[2].x + ( c+s)*radius, corners[2].y + (-c+s)*radius}, 1),
		square({corners[3].x + ( c+s)*radius, corners[3].y + ( c-s)*radius}, 2),
		square({corners[4].x + (-c-s)*radius, corners[4].y + (-c+s)*radius}, 2),
		square({corners[5].x + (-c-s)*radius, corners[5].y + ( c-s)*radius}, 1),
	};

	return result;
}

Rectangle const Stage { -1.0, -0.5, 1.0, 0.5 };

void setup(Window& window) {
	window.vertices() = {
	//	   X     Y     Z
	  	 0.5,  0.5,  0.0, // 0
	  	 0.5, -0.5,  0.0, // 1
	  	-0.5, -0.5,  0.0, // 2
	  	-0.5,  0.5,  0.0, // 3
	};

	window.indices() = {
		0, 1, 3,
		1, 2, 3,
	};
}

void renderLoop(Window& window) {
	auto* handler = window.handler();
	if (glfwGetKey(handler, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(handler, true);
	}

	glClearColor(0.9, 0.9, 0.9, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glUseProgram(window.shaderProgram());
	glBindVertexArray(window.VAO());
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	glfwSwapBuffers(handler);
	glfwPollEvents();
}

int main(int argc, char const* argv[]) {
	Window {
		800, 800,
		"Hello, 2x2x2!",
		setup,
		renderLoop
	};

	return EXIT_SUCCESS;
}
