#pragma once
#include "util.hh"
#include "math.hh"

template <class T>
concept IsPuzzle = std::regular<T> && requires(
	T const state, T modifiable,
	T::Move move
) {
	// The only possible way the user can modify our puzzle's state is by
	// picking it up, and giving it a twist :)
	modifiable.turn(move);

	// Every move advertised by our puzzle is allowed to be tried. It's just
	// that bandaged puzzles will sometimes "no nothing".
	{ state.moves() } -> IsRangeOf<typename T::Move>;

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
	// Index denotes position, value determines sticker.
	// So if state[20] == 10, position #20 is occupied by sticker #10.
	std::array<unsigned, 24> state_m {
		 0,  1,  2,  3,
		 4,  5,  6,  7,
		 8,  9, 10, 11,
		12, 13, 14, 15,
		16, 17, 18, 19,
		20, 21, 22, 23,
	};

	// Moves are to be constructed by the user.
	struct Move_t {
		unsigned face {};

		auto appearance() const {
			Plane3 result {};
			result = std::array<Plane3, 6> {{
				{{0, 0, 0}, {-1, 0, 0}},
				{{0, 0, 0}, {+1, 0, 0}},
				{{0, 0, 0}, {0, 0, -1}},
				{{0, 0, 0}, {0, 0, +1}},
				{{0, 0, 0}, {0, -1, 0}},
				{{0, 0, 0}, {0, +1, 0}},
			}} [face];
			return result;
		}
	};

	// Sticker data is constructed privately, their getters are public though.
	class Sticker_t {
		friend Cube2x2x2;
		unsigned position_m {}, id_m {};
		Sticker_t() = default;
		Sticker_t(unsigned position, unsigned id)
		:	position_m{position}, id_m{id} {}

	public:
		unsigned position() const { return position_m; }
		unsigned face()     const { return position_m / 4; }
		unsigned id()       const { return id_m; }
		unsigned color()    const { return id_m / 4; }

		auto polygon() const {
			auto result = std::array<Coord3, 4> {};
			// https://www.desmos.com/calculator/6pqhh5uipy
			result = std::array<std::array<Coord3, 4>> {{
				{{{-1,  0,  0}, {-1,  0, +1}, {-1, +1, +1}, {-1, +1,  0}}},
				{{{-1,  0,  0}, {-1, +1,  0}, {-1, +1, -1}, {-1,  0, -1}}},
				{{{-1,  0,  0}, {-1, -1,  0}, {-1, -1, +1}, {-1,  0, +1}}},
				{{{-1,  0,  0}, {-1,  0, -1}, {-1, -1, -1}, {-1, -1,  0}}},

				{{{+1,  0,  0}, {+1,  0, +1}, {+1, -1, +1}, {+1, -1,  0}}},
				{{{+1,  0,  0}, {+1, -1,  0}, {+1, -1, -1}, {+1,  0, -1}}},
				{{{+1,  0,  0}, {+1, +1,  0}, {+1, +1, +1}, {+1,  0, +1}}},
				{{{+1,  0,  0}, {+1,  0, -1}, {+1, +1, -1}, {+1, +1,  0}}},

				{{{ 0,  0, +1}, { 0, +1, +1}, {-1, +1, +1}, {-1,  0, +1}}},
				{{{ 0,  0, +1}, {-1,  0, +1}, {-1, -1, +1}, { 0, -1, +1}}},
				{{{ 0,  0, +1}, { 0, -1, +1}, {+1, -1, +1}, {+1,  0, +1}}},
				{{{ 0,  0, +1}, {+1,  0, +1}, {+1, +1, +1}, { 0, +1, +1}}},

				{{{ 0,  0, -1}, {-1,  0, -1}, {-1, +1, -1}, { 0, +1, -1}}},
				{{{ 0,  0, -1}, { 0, -1, -1}, {-1, -1, -1}, {-1,  0, -1}}},
				{{{ 0,  0, -1}, {+1,  0, -1}, {+1, -1, -1}, { 0, -1, -1}}},
				{{{ 0,  0, -1}, { 0, +1, -1}, {+1, +1, -1}, {+1,  0, -1}}},

				{{{ 0, +1,  0}, {-1, +1,  0}, {-1, +1, +1}, { 0, +1, +1}}},
				{{{ 0, +1,  0}, { 0, +1, -1}, {-1, +1, -1}, {-1, +1,  0}}},
				{{{ 0, +1,  0}, { 0, +1, +1}, {+1, +1, +1}, {+1, +1,  0}}},
				{{{ 0, +1,  0}, {+1, +1,  0}, {+1, +1, -1}, { 0, +1, -1}}},

				{{{ 0, -1,  0}, { 0, -1, +1}, {-1, -1, +1}, {-1, -1,  0}}},
				{{{ 0, -1,  0}, {-1, -1,  0}, {-1, -1, -1}, { 0, -1, -1}}},
				{{{ 0, -1,  0}, {+1, -1,  0}, {+1, -1, +1}, { 0, -1, +1}}},
				{{{ 0, -1,  0}, { 0, -1, -1}, {+1, -1, -1}, {+1, -1,  0}}},
			}} [position_m];
			return result;
		};
	};

	// Every 2x2x2 has unchanging properties. Because we only want to store
	// essential state in this class, properties_m is a static/global variable.
	// For run-time generated puzzles singletons can be used instead.
	struct Properties {
		std::array<Move_t, 6> moves {};
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
	using Move = Move_t;
	using Sticker = Sticker_t;
	auto operator<=>(Cube2x2x2 const&) const = default;

	void turn(Move move) {
		properties_m.permutations[move.face].shuffle(state_m);
	}

	auto const& moves() const { return properties_m.moves; }

	auto appearance() const {
		auto result = std::array<Sticker, 24> {};
		for (unsigned i=0; i<24; i++) {
			result[i] = Sticker {i, state_m[i]};
		}
		return result;
	}

	bool solved() const {
		using Color = unsigned;
		std::array<std::vector<Color>, 6> faces {};
		for (auto sticker: appearance()) {
			auto& face = faces[sticker.face()];
			auto color = sticker.color();
			if (face.size() > 1 && face.back() != color) {
				return false;
			}
			else face.push_back(color);
		}
		return true;
	};
};
