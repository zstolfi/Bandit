#pragma once
#include "util.hh"
#include "math.hh"

template <class T>
concept IsPuzzle = std::regular<T> && requires(
	T puzzle, T const puzzleState,
	T::Action action
) {
	// The user will always able to construct an action.
	{ action } -> std::regular;

	// The only possible way the user can modify our puzzle's state is by
	// picking it up, and giving it a twist :)
	puzzle.perform(action);

	// As per the name of this software, moves can be bandaged. We return a
	// falsy value if the user's action would do nothing.
	{ puzzleState.performable(action) } -> std::convertible_to<bool>;

	// Multiple solutions can be implemented by returning an object with an
	// overload for operator bool().
	{ puzzleState.solved() } -> std::convertible_to<bool>;
};

struct PuzzleSpec {/* TODO */};

class TwistyPuzzle {
	// Fundamentally, all twisty puzzles are permutable sets of stickers. You
	// might think it's more correct to describe them as permutable and
	// orientable cubies, but it is mathematically simpler (and therefore
	// less error prone) to describe them by stickers alone.

	// As we are not constraining ourselves to the physical world, the
	// engineering of digital twisty puzzle mechanisms is not bounded by
	// reality. For example, puzzles with more possible sticker positions than
	// stickers (i.e. shape-shifting puzzles or super cubes) are implemented
	// with an "invisible" sticker color.
	using ColorIndex_t = unsigned;
	std::vector<ColorIndex_t> state_m {};

	// Every twisty puzzle specification gives rise to an unchanging set of
	// properties. Because we'd like to copy around puzzle objects cheaply,
	// we share these properties with any copies of ourselves.
	struct Properties_t {
		friend TwistyPuzzle;
		decltype(TwistyPuzzle::state_m) solution {};
		std::vector<Permutation> moves {};
		Properties_t(PuzzleSpec const&);
	};
	std::shared_ptr<Properties_t const> properties_m {};

	// Move objects are just fancy indices, and do not contain permutation data.
	struct Move_t {
		unsigned index {};
		signed amount {1};
	};

public:
// Puzzle Interface
// ----------------
	using Action = Move_t;

	TwistyPuzzle() = default;

	TwistyPuzzle(PuzzleSpec const& spec) {
		properties_m = std::make_shared<Properties_t>(spec);
		state_m = properties_m->solution;
	}

	void perform(Move_t move) {
		permutation(move).apply_inplace(state_m);
		canonize();
	}

	bool performable(Move_t move) const {
		if (move.index >= properties_m->moves.size()) return false;
		// Bandaging logic will go here.
		return true;
	}

	bool solved() const {
		return state_m == properties_m->solution;
	};

	bool operator==(TwistyPuzzle const&) const = default;
	auto operator<=>(TwistyPuzzle const&) const = default;

private:
	Permutation permutation(Move_t move) const;

	void canonize() {/* Do nothing, for now. */}
};
