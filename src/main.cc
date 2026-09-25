#include "puzzle.hh"
#include "util.hh"
#include "window.hh"

auto const Colors = std::vector<std::array<float, 3>> {
	{{0.9, 0.5, 0.2}}, // Orange
	{{0.9, 0.2, 0.2}}, // Red
	{{0.9, 0.9, 0.9}}, // White
	{{0.9, 0.9, 0.2}}, // Yellow
	{{0.2, 0.3, 0.9}}, // Blue
	{{0.1, 0.9, 0.2}}, // Green
};

auto const ColorsBackground = std::vector<std::array<float, 3>> {
	{{0.7, 0.7, 0.7}}, // Unsolved
	{{0.6, 0.7, 0.6}}, // Solved
};

/* ~~ Application State ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

struct AppState {
	std::vector<std::string_view> args {};
	std::vector<GLfloat> vertices {};
	std::vector<GLuint> indices {};
	std::map<unsigned, std::vector<unsigned>> stickerIndices {};

	using Puzzle = Cube2x2x2;
	Puzzle puzzle {};
	bool solved {}, update {true};

	struct KeyInfo { bool state {}; signed delta {}; };
	std::map<int, KeyInfo> keys {};
};

using AppWindow = Window<AppState>;

void setup(AppWindow& window, AppState& state) {
	// Create our puzzle's geometry.
	GLuint index = 0;
	for (auto sticker: state.puzzle.appearance()) {
		auto const& polygon = sticker.polygon();
		auto const& color = Colors[sticker.color()];
		bool show = sticker.color() == 0;
		for (Coord3 const& c: polygon) {
			state.stickerIndices[sticker.position()].push_back(index);
			state.vertices.push_back(show? -c.y(): 0.0);
			state.vertices.push_back(show?  c.z(): 0.0);
			state.vertices.push_back(/*show? c.z():*/ 0.0);
			state.vertices.push_back(color[0]);
			state.vertices.push_back(color[1]);
			state.vertices.push_back(color[2]);
			index++;
		}
		state.indices.push_back(index-4 + 0);
		state.indices.push_back(index-4 + 1);
		state.indices.push_back(index-4 + 2);
		state.indices.push_back(index-4 + 2);
		state.indices.push_back(index-4 + 3);
		state.indices.push_back(index-4 + 0);
	}

	window.vertices(state.vertices);
	window.indices(state.indices);

	// Keep track of which keys been pressed.
	window.bind(glfwSetKeyCallback,
		[&] (int key, int scancode, int action, int mods) {
			if (action == GLFW_PRESS) state.keys[key] = {true, 1};
			if (action == GLFW_RELEASE) state.keys[key] = {false, -1};
		}
	);
};

/* ~~ Input Handling ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void processInput(AppWindow& window, AppState& state) {
	auto turn = [&] (unsigned index, unsigned times=1) {
		auto move = state.puzzle.moves()[index];
		while (times--) state.puzzle.turn(move);
	};

	static std::map<int, std::function<void ()>> const keyMap {
		// Exit.
		{GLFW_KEY_ESCAPE, [&] { window.close(); state.update = false; } },

		// Turn front upper-left.
		{GLFW_KEY_E, [&] { turn(4   ); }}, // L'
		{GLFW_KEY_D, [&] { turn(4, 3); }}, // L
		{GLFW_KEY_F, [&] { turn(2   ); }}, // U'
		{GLFW_KEY_S, [&] { turn(2, 3); }}, // U
		{GLFW_KEY_W, [&] { turn(0   ); }}, // F'
		{GLFW_KEY_R, [&] { turn(0, 3); }}, // F

		// Turn back lower-right.
		{GLFW_KEY_I, [&] { turn(5, 3); }}, // R
		{GLFW_KEY_K, [&] { turn(5   ); }}, // R'
		{GLFW_KEY_L, [&] { turn(3, 3); }}, // D
		{GLFW_KEY_J, [&] { turn(3   ); }}, // D'
		{GLFW_KEY_U, [&] { turn(1, 3); }}, // B
		{GLFW_KEY_O, [&] { turn(1   ); }}, // B'
	};

	// Apply action based on any detected key presses.
	for (auto& [key, keyInfo]: state.keys) {
		if (keyInfo.delta == 1) {
			state.update = true;
			auto entry = keyMap.find(key);
			if (entry != keyMap.end()) entry->second();
		}
		// Rest any "pulse" signal after observing.
		keyInfo.delta = 0;
	}
};

/* ~~ Rendering ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void renderLoop(AppWindow& window, AppState& state) {
	processInput(window, state);
	if (state.update) {
		state.update = false;
		state.solved = state.puzzle.solved();

		// Update the puzzle's display.
		for (auto sticker: state.puzzle.appearance()) {
			auto color = Colors[sticker.color()];
			auto position = sticker.position();
			for (unsigned index: state.stickerIndices[position]) {
				state.vertices[6 * index + 3] = color[0];
				state.vertices[6 * index + 4] = color[1];
				state.vertices[6 * index + 5] = color[2];
			}
		}

		// Re-send vertex data. I'm pretty sure this is the wrong way of
		// updating the display of the puzzle. I think the proper solution is
		// to use the vertex shader to update each sticker's position.
		window.vertices(state.vertices);
	}

	// Clear our window's screen.
	auto const& bg = ColorsBackground[state.solved];
	glClearColor(bg[0], bg[1], bg[2], 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	// Draw our beautiful puzzle.
	glUseProgram(window.shaderProgram());
	glBindVertexArray(window.VAO());
	glDrawElements(GL_TRIANGLES, state.indices.size(), GL_UNSIGNED_INT, {});
	glBindVertexArray(0);

	// Get ready for the next frame.
	glfwSwapBuffers(window.handle());
	glfwPollEvents();
};

/* ~~ Main Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int main(int argc, char const* argv[]) {
	std::vector<std::string_view> args {argv, argv+argc};
	AppWindow {
		800, 800,
		"Hello, 2x2x2!",
		setup,
		renderLoop,
		AppState {args}
	};

	return EXIT_SUCCESS;
}
