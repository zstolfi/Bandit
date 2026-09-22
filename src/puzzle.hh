#pragma once
#include "util.hh"

class Permutation {
	std::vector<unsigned> m_word {};

public:
	Permutation() = default;

	Permutation(std::vector<unsigned> word) {
		// Take notes on user input.
		unsigned largest = 0;
		std::set<unsigned> included {};
		for (unsigned i : word) {
			largest = std::max(largest, i);
			included.insert(i);
		}

		// Words contain every element smaller than their size exactly once.
		if (largest >= word.size() || included.size() != word.size()) {
			throw std::invalid_argument {"malformed word"};
		}

		// Construct.
		m_word = word;
		normalize();
	}

	Permutation(std::vector<std::vector<unsigned>> cycles) {
		// Take notes on user input.
		unsigned largest = 0;
		std::map<unsigned, unsigned> counts {};
		for (auto const& cycle : cycles) {
			for (unsigned i : cycle) {
				largest = std::max(largest, i);
				counts[i]++;
			}
		}

		// Allow the identity permutation.
		if (largest == 0) return;
		// Elements cannot be repeated within or between cycles
		for (unsigned count : counts | stdv::values) if (count != 1) {
			throw std::invalid_argument {"malformed cycles"};
		}

		// Construct "word" form of permutation.
		m_word.resize(largest+1);
		stdr::iota(m_word, 0);
		for (auto& cycle : cycles) {
			unsigned temp = m_word[cycle.front()];
			for (unsigned i=0; i<cycle.size()-1; i++) {
				m_word[cycle[i]] = m_word[cycle[i+1]];
			}
			m_word[cycle.back()] = temp;
		}
		normalize();
	}

	auto operator<=>(Permutation const&) const = default;

	unsigned operator[](unsigned i) const {
		if (i < m_word.size()) return m_word[i];
		else return i;
	}

private:
	void normalize() {
		while (!m_word.empty() && m_word.back() == m_word.size()-1) {
			m_word.pop_back();
		}
	}
};

template <class T>
concept Puzzle = std::regular<T> && requires(T const state, T modifiable) {
	// Bandaging is implemented by allowing moves to sometimes do nothing. That
	// is, depending on state, they perform the identity permutation.
	{ state.moves() } -> RangeOf<typename T::MoveIndex>;
	{ state.bandaged(typename T::MoveIndex {}) } -> std::convertible_to<bool>;

	// Puzzle state is stored in terms of cubies (unions of stickers). For
	// example, a 2x2x2 has 8 cubies, each with 24 possible orientations.
	{ state.cubies() } -> RangeOf<typename T::Cubie>;

	// Stickers, as the only component with color, are what determine if we are
	// solved or not.
	{ state.stickers() } -> RangeOf<typename T::Sticker>;
	{ state.solved() } -> std::convertible_to<bool>;

	// The only possible way the user can modify our puzzle's state is by
	// picking it up, and giving it a twist :)
	{ modifiable.turn(typename T::MoveIndex {}) };
};

// 2x2x2
// https://www.desmos.com/calculator/6pqhh5uipy
class Cube2x2x2 {
public:
	using Orientation = unsigned;
	using Color = unsigned;

	class Sticker {
		friend Cube2x2x2;
		Orientation m_orientation {};
		Color m_color {};

		Sticker (Orientation o, Color c)
		:	m_orientation{o}, m_color{c} {}

	public:
		auto operator<=>(Sticker const&) const = default;
		auto const& orientation() const { return m_orientation; }
		auto const& color() const { return m_color; }
	};

	class Move : public Permutation {
		friend Cube2x2x2;
		using Permutation::Permutation;
	};

private:
	std::vector<Sticker> m_stickers {
		{ 0, 0}, { 1, 0}, { 2, 0}, { 3, 0},
		{ 4, 1}, { 5, 1}, { 6, 1}, { 7, 1},
		{ 8, 2}, { 9, 2}, {10, 2}, {11, 2},
		{12, 3}, {13, 3}, {14, 3}, {15, 3},
		{16, 4}, {17, 4}, {18, 4}, {19, 4},
		{20, 5}, {21, 5}, {22, 5}, {23, 5},
	};

	std::vector<Move> m_moves {
		{{{ 0,  1,  3,  2}, { 8, 17, 13, 20}, {16, 12, 21,  9}}}, // F'
		{{{ 5,  7,  6,  4}, {14, 19, 11, 22}, {23, 15, 18, 10}}}, // B'
		{{{ 8,  9, 10, 11}, { 0, 20,  4, 18}, {16,  2, 22,  6}}}, // U'
		{{{14, 13, 12, 15}, { 5, 21,  1, 19}, {23,  3, 17,  7}}}, // D'
		{{{16, 18, 19, 17}, { 0, 11,  7, 12}, { 8,  6, 15,  1}}}, // L'
		{{{23, 22, 20, 21}, { 5, 10,  2, 13}, {14,  4,  9,  3}}}, // R'
	};

public:
	Cube2x2x2() = default;
	auto operator<=>(Cube2x2x2 const&) const = default;

	using MoveIndex = Move;
	auto const& moves() const { return m_moves; }
	bool bandaged(Move const&) const { return false; }

	using Cubie = std::vector<unsigned>;
	auto cubies() const {
		auto result = std::vector<Cubie> {};
		return result = {
			{ 0,  8, 16}, { 1, 17, 12}, { 2, 20,  9}, { 3, 13, 21},
			{ 4, 10, 22}, { 5, 23, 14}, { 6, 18, 11}, { 7, 15, 19},
		};
	}

	auto const& stickers() const { return m_stickers; }

	bool solved() const {
		std::array<std::vector<Sticker>, 6> faces {};
		for (auto const& sticker : m_stickers) {
			auto& face = faces[sticker.orientation() / 4];
			if (face.size() > 1 && face.back().color() != sticker.color()) {
				return false;
			}
			else face.push_back(sticker);
		}
		return true;
	}

	void turn(Move const& move) {
		stdr::for_each(m_stickers, [&] (auto& sticker) {
			sticker.m_orientation = move[sticker.m_orientation];
		});
	}
};
