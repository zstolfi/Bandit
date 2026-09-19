#include "puzzle.hh"
#include "util.hh"
#include "window.hh"

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
	float const radius = 0.1 * pieceSize;
	float const s = 0.5, c = 0.5 * std::sqrt(3);
	float const a = s+c, b = s-c;

	auto square = [&] (Coord v, int angle) {
		auto result = std::array<Coord, 4> {};
		/**/ if (angle == 0) result = {{{1, 1}, {-1, 1}, {-1, -1}, {1, -1}}};
		else if (angle == 1) result = {{{a, b}, {-b, a}, {-a, -b}, {b, -a}}};
		else if (angle == 2) result = {{{b, a}, {-a, b}, {-b, -a}, {a, -b}}};
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

std::vector<std::array<float, 3>> colors {
	{{0.9, 0.9, 0.9}}, // White
	{{0.9, 0.9, 0.2}}, // Yellow
	{{0.9, 0.2, 0.2}}, // Red
	{{0.9, 0.5, 0.2}}, // Orange
	{{0.2, 0.3, 0.9}}, // Blue
	{{0.1, 0.9, 0.2}}, // Green
};

Puzzle puzzle {};
bool solved {};

void setup(Window& window) {
	GLuint index = 0;
	auto const displaySquares = squarePositions(0.95);
	for (auto const& sticker : puzzle.stickers()) {
		for (Coord const& c : displaySquares[sticker.orientation()]) {
			window.vertices().push_back(c.x);
			window.vertices().push_back(c.y);
			window.vertices().push_back(0.0);
			window.vertices().push_back(colors[sticker.color()][0]);
			window.vertices().push_back(colors[sticker.color()][1]);
			window.vertices().push_back(colors[sticker.color()][2]);
		}
		window.indices().push_back(4 * index + 0);
		window.indices().push_back(4 * index + 1);
		window.indices().push_back(4 * index + 2);
		window.indices().push_back(4 * index + 2);
		window.indices().push_back(4 * index + 3);
		window.indices().push_back(4 * index + 0);
		index++;
	}
}

void processInput(Window& window) {
	auto* handler = window.handler();
	if (glfwGetKey(handler, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(handler, true);
	}

	if (glfwGetKey(handler, GLFW_KEY_A) == GLFW_PRESS) {
		solved = puzzle.solved();
	}
}

void renderLoop(Window& window) {
	auto* handler = window.handler();
	processInput(window);

	if (solved) glClearColor(0.5, 0.7, 0.5, 1.0);
	else glClearColor(0.7, 0.7, 0.7, 1.0);


	glClear(GL_COLOR_BUFFER_BIT);

//	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	GLint u_color = glGetUniformLocation(window.shaderProgram(), "u_color");
	glUseProgram(window.shaderProgram());

	float time = glfwGetTime();
	float wave = 0.5 + 0.5 * std::sin(time);
	glUniform4f(u_color, 0.0, wave, 0.0, 1.0);

	glBindVertexArray(window.VAO());
	glDrawElements(
		GL_TRIANGLES,
		window.indices().size(),
		GL_UNSIGNED_INT, {}
	);
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
