#pragma once
#include "util.hh"
#include "math.hh"

template <class T>
concept PuzzleType = std::regular<T> && requires(
	T const state, T modifiable,
	typename T::Move move
) {
	// The only possible way the user can modify our puzzle's state is by
	// picking it up, and giving it a twist :)
	modifiable.turn(move);

	// Every move advertised by our puzzle is allowed to be tried. It's just that
	// bandaged puzzles will sometimes "no nothing".
	{ state.moves() } -> RangeOf<typename T::Move>;

	// We can report back any appearance information we'd like. It's usually a
	// good idea to make solved() depend on this data.
	state.appearance();

	// Multiple solutions can be implemented by returning an object which an
	// overload for operator bool().
	{ state.solved() } -> std::convertible_to<bool>;
};

// 2x2x2 Rubik's Cube
// https://www.desmos.com/calculator/6pqhh5uipy
class Cube2x2x2 {
	// Index denotes orientation, value determines sticker color.
	// So if state[20] == 2, orientation #20 is occupied by sticker #2.
	std::array<unsigned, 24> state_m {
		0, 0, 0, 0,
		1, 1, 1, 1,
		2, 2, 2, 2,
		3, 3, 3, 3,
		4, 4, 4, 4,
		5, 5, 5, 5,
	};

	struct Properties {
		struct Move { unsigned face {}; };
		std::array<Move, 6> moves {};
		std::array<Permutation, 6> permutations {};

		Properties()
		:	moves {{{0}, {1}, {2}, {3}, {4}, {5}}}
		,	permutations {{
				{{{ 0,  2,  3,  1}, { 8, 20, 13, 17}, {16,  9, 21, 12}}}, // F'
				{{{ 5,  4,  6,  7}, {14, 22, 11, 19}, {23, 10, 18, 15}}}, // B'
				{{{ 8, 11, 10,  9}, { 0, 18,  4, 20}, {16,  6, 22,  2}}}, // U'
				{{{14, 15, 12, 13}, { 5, 19,  1, 21}, {23,  7, 17,  3}}}, // D'
				{{{16, 17, 19, 18}, { 0, 12,  7, 11}, { 8,  1, 15,  6}}}, // L'
				{{{23, 21, 20, 22}, { 5, 13,  2, 10}, {14,  3,  9,  4}}}, // R'
			}}
		{}
	};

	static inline Properties const properties_m {};

public:
	using Move = Properties::Move;
	auto operator<=>(Cube2x2x2 const&) const = default;

	void turn(Move move) {
		properties_m.permutations[move.face].shuffle(state_m);
	}

	auto const& moves() const { return properties_m.moves; }

	auto appearance() const {
		struct Sticker {
			unsigned orientation {};
			unsigned color {};
		};
		auto result = std::array<Sticker, 24> {};

		for (unsigned i=0; i<24; i++) {
			result[i] = Sticker {i, state_m[i]};
		}

		return result;
	};

	bool solved() const {
		using Color = unsigned;
		std::array<std::vector<Color>, 6> faces {};
		for (auto [orientation, color] : appearance()) {
			auto& face = faces[orientation / 4];
			if (face.size() > 1 && face.back() != color) {
				return false;
			}
			else face.push_back(color);
		}
		return true;
	};
};
