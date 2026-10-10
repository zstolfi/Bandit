#include "graphics.hh"
#include "puzzle.hh"
#include "util.hh"
#include "window.hh"

/* ~~ Application State ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

struct AppState {
	std::vector<std::string_view> args {};

	using Puzzle = Cube2x2x2;
	Puzzle puzzle {};
	Model<Puzzle> model {};
	bool solved {}, update {true};

	struct KeyInfo { bool state {}; signed delta {}; };
	std::map<int, KeyInfo> keys {};
};

using AppWindow = Window<AppState>;

void setup(AppWindow& window, AppState& state) {
	window.vertices(state.model.vertices());

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
		while (times--) state.puzzle.apply(move);
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

		state.model.update(state.puzzle);
		window.vertices(state.model.vertices());
	}

	// Clear our window's screen.
	auto const& bg = ColorsBackground[state.solved];
	glClearColor(bg[0], bg[1], bg[2], 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Draw our beautiful puzzle.
	glUseProgram(window.shaderProgram());

	double const fov = 0.1 * TWO_PI;
	double const f = std::tan(TWO_PI/4 - fov/2);
	auto const [width, height] = window.size();
	double const aspect = 1.0 * width / height;
	double const near = 0.1, far = 100.0;
	// https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml
	using Matx4f = Matx<4, 4, float>;
	Matx4f projection {{
		f/aspect, 0, 0, 0,
		0, f, 0, 0,
		0, 0, (near + far)/(near - far), (2 * near * far)/(near - far),
		0, 0, -1, 0,
	}};

	Coord3 const position {0, 0, -5};
	Matx4f view {{
		1, 0, 0, position.x(),
		0, 1, 0, position.y(),
		0, 0, 1, position.z(),
		0, 0, 0, 1,
	}};

//	if (auto const& mouse = window.mouse()) {
//		float const panX = 1.0 * (*mouse)[0] / window.size()[0] - 0.5;
//		float const panY = 1.0 * (*mouse)[1] / window.size()[1] - 0.5;
//		view *= Matx4::RotateX(-4.0 * panY);
//		view *= Matx4::RotateY( 6.0 * panX);
//	}

	Matx4f model {};

	window.uniform(glUniformMatrix4fv,
		"projection",
		1, GL_TRUE, projection.pointer()
	);
	window.uniform(glUniformMatrix4fv,
		"view",
		1, GL_TRUE, view.pointer()
	);
	window.uniform(glUniformMatrix4fv,
		"model",
		1, GL_TRUE, model.pointer()
	);

	glBindVertexArray(window.VAO());
	glDrawArrays(GL_TRIANGLES, 0, state.model.vertices().size()/6);
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
