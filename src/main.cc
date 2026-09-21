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
bool solved {}, update {true};

struct KeyInfo { bool state {}; signed delta {}; };
std::map<int, KeyInfo> keys {};

void setup(Window& window) {
	// Puzzle display:
	GLuint index = 0;
	for (auto const& square : squarePositions(0.95)) {
		for (Coord const& c : square) {
			// This is very inefficient.
			window.vertices().push_back(c.x);
			window.vertices().push_back(c.y);
			window.vertices().push_back(0.0);
			window.vertices().push_back(colors[index / 4][0]);
			window.vertices().push_back(colors[index / 4][1]);
			window.vertices().push_back(colors[index / 4][2]);
		}
		window.indices().push_back(4 * index + 0);
		window.indices().push_back(4 * index + 1);
		window.indices().push_back(4 * index + 2);
		window.indices().push_back(4 * index + 2);
		window.indices().push_back(4 * index + 3);
		window.indices().push_back(4 * index + 0);
		index++;
	}

	// Puzzle controls:
	glfwSetKeyCallback(window.handler(),
		[] (GLFWwindow*, int key, int scancode, int action, int mods) {
			/**/ if (action == GLFW_PRESS) keys[key] = {true, 1};
			else if (action == GLFW_RELEASE) keys[key] = {false, -1};
		}
	);
}

void processInput(Window& window) {
	auto turn = [&] (unsigned index, unsigned times=1) {
		while (times--) puzzle.turn(puzzle.moves()[index]);
	};

	static std::map<int, std::function<void ()>> const keyMap {
		{GLFW_KEY_ESCAPE,
			[&] { glfwSetWindowShouldClose(window.handler(), true); }
		},

		{GLFW_KEY_E, [&] { turn(4   ); }}, // L'
		{GLFW_KEY_D, [&] { turn(4, 3); }}, // L
		{GLFW_KEY_F, [&] { turn(2   ); }}, // U'
		{GLFW_KEY_S, [&] { turn(2, 3); }}, // U
		{GLFW_KEY_W, [&] { turn(0   ); }}, // F'
		{GLFW_KEY_R, [&] { turn(0, 3); }}, // F

		{GLFW_KEY_I, [&] { turn(5, 3); }}, // R
		{GLFW_KEY_K, [&] { turn(5   ); }}, // R'
		{GLFW_KEY_L, [&] { turn(3, 3); }}, // D
		{GLFW_KEY_J, [&] { turn(3   ); }}, // D'
		{GLFW_KEY_U, [&] { turn(1, 3); }}, // B
		{GLFW_KEY_O, [&] { turn(1   ); }}, // B'
	};

	for (auto& [key, keyInfo] : keys) {
		if (keyInfo.delta == 1) {
			update = true;
			auto entry = keyMap.find(key);
			if (entry != keyMap.end()) entry->second();
		}
		keyInfo.delta = 0;
	}
}

void renderLoop(Window& window) {
	processInput(window);
	if (update) {
		update = false;
		solved = puzzle.solved();

		// Puzzle display:
		for (auto const& sticker : puzzle.stickers()) {
			auto color = colors[sticker.color()];
			for (unsigned j=0; j<4; j++) {
				auto index = 4 * sticker.orientation() + j;
				window.vertices()[6 * index + 3] = color[0];
				window.vertices()[6 * index + 4] = color[1];
				window.vertices()[6 * index + 5] = color[2];
			}
		}

		// Re-send vertex data. I'm pretty sure this is the wrong way of
		// updating the display of the puzzle. I think the proper solution is
		// to use the vertex shader to update each sticker's orientation.
		glBindBuffer(GL_ARRAY_BUFFER, window.VBO());
		glBufferData(
			GL_ARRAY_BUFFER,
			window.vertices().size() * sizeof(GLfloat),
			window.vertices().data(),
			GL_STATIC_DRAW
		);
	}

	if (solved) glClearColor(0.6, 0.7, 0.6, 1.0);
	else glClearColor(0.7, 0.7, 0.7, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(window.shaderProgram());
	glBindVertexArray(window.VAO());
	glDrawElements(
		GL_TRIANGLES,
		window.indices().size(),
		GL_UNSIGNED_INT, {}
	);
	glBindVertexArray(0);

	glfwSwapBuffers(window.handler());
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
