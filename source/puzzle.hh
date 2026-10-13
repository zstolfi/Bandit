#pragma once
#include "util.hh"
#include "math.hh"

struct PuzzleSpec {/* TODO */};

template <class T>
concept IsPuzzleState = std::regular<T> && requires(
	T modifiable, T const state,
	T::Action action
) {
	// Actions, provided by the user, are the only way our state is allowed to
	// be modified.
	{ action } -> std::regular;
	modifiable.perform(action);

	// Sometimes we want to know if an action is valid without trying to do it.
	// This method is also used for implementing bandage logic.
	{ state.performable(action) } -> std::convertible_to<bool>;

	// Every puzzle has at least one solution, otherwise it's a toy. Multiple
	// can be provided by returning an object with a operator bool() overload.
	{ state.solved() } -> std::convertible_to<bool>;
};

class TwistyPuzzleState {
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
		friend TwistyPuzzleState;
		decltype(TwistyPuzzleState::state_m) solution {};
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

	TwistyPuzzleState() = default;

	TwistyPuzzleState(PuzzleSpec const& spec) {
		properties_m = std::make_shared<Properties_t>(spec);
		state_m = properties_m->solution;
	}

	void perform(Move_t move) {
		if (move.amount == 0) return;
		signed direction = (move.amount < 0)? -1: 1;
		unsigned times = move.amount * direction;

		auto permutation = properties_m->moves[move.index];
		if (backwards) permutation = permutation.inverse();

		while (times-- && performable({move.index, direction})) {
			permutation.apply_inplace(state_m);
		}
		canonize();
	}

	bool performable(Move_t move) const {
		if (move.index >= properties_m->moves.size()) return false;
		if (move.amount == 0) return false;
		// Bandaging logic will go here.
		return true;
	}

	bool solved() const {
		return state_m == properties_m->solution;
	};

	bool operator==(TwistyPuzzleState const&) const = default;
	auto operator<=>(TwistyPuzzleState const&) const = default;

private:
	void canonize() {/* Do nothing, for now. */}
};
