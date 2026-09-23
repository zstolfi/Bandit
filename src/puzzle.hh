#pragma once
#include "util.hh"

class Permutation {
	using Cycle = std::vector<unsigned>;
	std::vector<Cycle> m_cycles {};

public:
	Permutation() = default;
	auto operator<=>(Permutation const&) const = default;

	Permutation(std::vector<Cycle> const& cycles) {
		// Take notes on user input.
		std::map<unsigned, unsigned> counts {};
		for (auto const& cycle : cycles) {
			for (unsigned i : cycle) {
				counts[i]++;
			}
		}
		// Elements cannot be repeated within or between cycles
		for (unsigned count : counts | stdv::values) if (count != 1) {
			throw std::invalid_argument {"malformed cycles"};
		}
		// Assign.
		m_cycles = cycles;
		normalize();
	}

	unsigned operator[](unsigned index) const {
		if (m_cycles.empty()) return index;
		if (index > m_cycles.back()[0]) return index;
		for (auto const& cycle : m_cycles) {
			for (unsigned i=0; i<cycle.size(); i++) {
				if (index == cycle[i]) {
					return (i+1 < cycle.size())
					?	cycle[i+1]
					:	cycle[0];
				}
			}
		}
		return index;
	}

	template <stdr::random_access_range Range>
	void apply(Range& input) const {
		for (auto const& cycle : m_cycles) {
			for (auto [i, j] : cycle | stdv::pairwise) {
				std::swap(input[i], input[j]);
			}
		}
	}

private:
	void normalize() {
		for (auto& cycle : m_cycles) {
			stdr::rotate(cycle, stdr::max_element(cycle));
		}
		stdr::sort(m_cycles, stdr::less {},
			[] (auto const& cycle) { return cycle[0]; }
		);
	}
};

template <class T>
concept Puzzle = std::regular<T> && requires(T const state, T modifiable) {
	// Bandaging is implemented by allowing moves to sometimes do nothing. That
	// is, depending on state, they perform the identity permutation.
	{ state.moves() } -> RangeOf<typename T::Move>;
	{ state.bandaged(typename T::Move {}) } -> std::convertible_to<bool>;

	// Stickers are the sole determiner of whether or not our puzzle is solved.
	{ state.stickers() } -> RangeOf<typename T::Sticker>;
	{ state.solved() } -> std::convertible_to<bool>;

	// The only possible way the user can modify our puzzle's state is by
	// picking it up, and giving it a twist :)
	{ modifiable.turn(typename T::Move {}) };
};

// 2x2x2
// https://www.desmos.com/calculator/6pqhh5uipy
class Cube2x2x2 {
public:
	enum struct OrientationIndex : unsigned {/* 0 - 23 */};
	enum struct StickerIndex     : unsigned {/* 0 - 23 */};
	enum struct FaceIndex        : unsigned {/* 0 - 5 */};
	enum struct ColorIndex       : unsigned {/* 0 - 5 */};

	struct Move {
		FaceIndex face {6};
	};

private:
	// 2x2x2 state is stored as a shuffling of the 24 stickers.
	std::array<StickerIndex, 24> m_state {{
		StickerIndex{ 0}, StickerIndex{ 1}, StickerIndex{ 2}, StickerIndex{ 3},
		StickerIndex{ 4}, StickerIndex{ 5}, StickerIndex{ 6}, StickerIndex{ 7},
		StickerIndex{ 8}, StickerIndex{ 9}, StickerIndex{10}, StickerIndex{11},
		StickerIndex{12}, StickerIndex{13}, StickerIndex{14}, StickerIndex{15},
		StickerIndex{16}, StickerIndex{17}, StickerIndex{18}, StickerIndex{19},
		StickerIndex{20}, StickerIndex{21}, StickerIndex{22}, StickerIndex{23},
	}};

public:
	Cube2x2x2() = default;
	auto operator<=>(Cube2x2x2 const&) const = default;

	auto moves() const {
		auto result = std::array<Move, 6> {};
		for (unsigned i=0; i<6; i++) result[i] = Move {FaceIndex(i)};
		return result;
	}

	bool bandaged(Move const&) const {
		return false;
	}

	bool solved() const {
		std::array<std::vector<Sticker>, 6> faces {};
		for (auto [orientation, sticker] : stdv::zip(stdv::iota(0), m_state)) {
			auto& face = faces[orientation / 4];
			if (face.size() > 1
			&&  stickerColor(face.back()) != stickerColor(sticker)) {
				return false;
			}
			else face.push_back(sticker);
		}
		return true;
	}

	using Sticker = StickerIndex;
	auto const& stickers() const { return m_state; }

	ColorIndex stickerColor(StickerIndex sticker) const {
		return ColorIndex(unsigned(sticker)/4);
	}

	struct Cubie {
		std::vector<Sticker> stickers {};
		Cubie(auto ... args): stickers{Sticker(args) ... } {}
	};
	auto cubies() const {
		auto result = std::vector<Cubie> {};
		return result = {
			{ 0,  8, 16}, { 1, 17, 12}, { 2, 20,  9}, { 3, 13, 21},
			{ 4, 10, 22}, { 5, 23, 14}, { 6, 18, 11}, { 7, 15, 19},
		};
	}

	void turn(Move move) {
		static std::array<Permutation, 6> const permutations = {{
			{{{ 0,  1,  3,  2}, { 8, 17, 13, 20}, {16, 12, 21,  9}}}, // F'
			{{{ 5,  7,  6,  4}, {14, 19, 11, 22}, {23, 15, 18, 10}}}, // B'
			{{{ 8,  9, 10, 11}, { 0, 20,  4, 18}, {16,  2, 22,  6}}}, // U'
			{{{14, 13, 12, 15}, { 5, 21,  1, 19}, {23,  3, 17,  7}}}, // D'
			{{{16, 18, 19, 17}, { 0, 11,  7, 12}, { 8,  6, 15,  1}}}, // L'
			{{{23, 22, 20, 21}, { 5, 10,  2, 13}, {14,  4,  9,  3}}}, // R'
		}};

		permutations[unsigned(move.face)].apply(m_state);
	}
};
