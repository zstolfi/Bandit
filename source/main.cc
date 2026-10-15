#include "graphics.hh"
#include "puzzle.hh"
#include "util.hh"
#include "window.hh"

/* ~~ Application State ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

PuzzleSpec const spec2x2x2 {
	.solution = {
		// We reserve 0 for "no color".
		1, 1, 1, 1,
		2, 2, 2, 2,
		3, 3, 3, 3,
		4, 4, 4, 4,
		5, 5, 5, 5,
		6, 6, 6, 6
	},
	.moves = {
		// One move per plane cut.
		Permutation {{ 4,  5,  6,  7}, { 9, 18, 12, 23}, {10, 19, 13, 20}},
		Permutation {{12, 13, 14, 15}, { 2, 20,  7, 17}, { 3, 21,  4, 18}},
		Permutation {{20, 21, 22, 23}, { 1, 10,  4, 15}, { 2, 11,  5, 12}},
	},
};

// In this software, PuzzleStates are implemented in a mathematically "nice"
// way. As such, we need another layer of abstraction to manage puzzles in a way
// that feels natural.
template <IsPuzzleState State_t>
class Puzzle {
	State_t state_m {};
	std::shared_ptr<Model> model_m {};

	// Statistics for our puzzle, so we don't have to call state_m every frame.
	bool solved_m {};

public:
	Puzzle() = default;

	Puzzle(auto const& ... args)
	:	state_m {args ... } { update(); }

	void turn(unsigned index, signed times=1) {
		// TODO: Add "observers", which allow the puzzle to do moves based on
		// face, and XYZ rotation moves.
		state_m.perform({index, times});
		update();
	};

	auto const& state() const { return state_m; }
	auto model() const { return model_m; }
	bool solved() const { return solved_m; }

private:
	void update() {
		// Update model here.
		solved_m = state_m.solved();
	}
};

struct AppState {
	std::vector<std::string_view> args {};
	Puzzle<TwistyPuzzleState> puzzle {};
	Scene mainScene {};

	// TOOD: Move into Window class, alongside mouse input.
	struct KeyInfo { bool state {}; signed delta {}; };
	std::map<int, KeyInfo> keys {};
};

using AppWindow = Window<AppState>;

void setup(AppWindow& window, AppState& state) {
	state.puzzle = {spec2x2x2};
	state.mainScene.models.push_back(state.puzzle.model());
	window.graphics().scene(state.mainScene);

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
	static std::map<int, std::function<void ()>> const keyMap {
		// Exit.
		{GLFW_KEY_ESCAPE, [&] { window.close(); } },

		// Turn front upper-left.
		{GLFW_KEY_E, [&] { state.puzzle.turn(2    ); }}, // L'
		{GLFW_KEY_D, [&] { state.puzzle.turn(2, -1); }}, // L
		{GLFW_KEY_F, [&] { state.puzzle.turn(1    ); }}, // U'
		{GLFW_KEY_S, [&] { state.puzzle.turn(1, -1); }}, // U
		{GLFW_KEY_W, [&] { state.puzzle.turn(0    ); }}, // F'
		{GLFW_KEY_R, [&] { state.puzzle.turn(0, -1); }}, // F

//		// Turn back lower-right.
//		{GLFW_KEY_I, [&] { turn(5, -1); }}, // R
//		{GLFW_KEY_K, [&] { turn(5    ); }}, // R'
//		{GLFW_KEY_L, [&] { turn(3, -1); }}, // D
//		{GLFW_KEY_J, [&] { turn(3    ); }}, // D'
//		{GLFW_KEY_U, [&] { turn(1, -1); }}, // B
//		{GLFW_KEY_O, [&] { turn(1    ); }}, // B'
	};

	// Apply action based on any detected key presses.
	for (auto& [key, keyInfo]: state.keys) {
		if (keyInfo.delta == 1) {
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
	state.mainScene.backgroundColor = ColorsBackground[state.puzzle.solved()];
	window.graphics().scene(state.mainScene);
	window.graphics().draw();
};

/* ~~ Main Function ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

int main(int argc, char const* argv[]) {
	std::vector<std::string_view> args {argv, argv+argc};
	AppWindow {
		800, 600,
		"Hello, 2x2x2!",
		setup,
		renderLoop,
		AppState {args}
	};

	return EXIT_SUCCESS;
}
