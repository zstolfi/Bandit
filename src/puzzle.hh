#pragma once
#include "util.hh"

class Permutation {
	std::vector<unsigned> m_word {};

public:
	Permutation(std::vector<std::vector<unsigned>> cycles) {
		// TODO: Detect of malformed cycles.
		m_word.resize(24);
		stdr::iota(m_word, 0);
		for (auto& cycle : cycles) {
			unsigned temp = m_word[cycle.front()];
			for (unsigned i=0; i<cycle.size()-1; i++) {
				m_word[cycle[i]] = m_word[cycle[i+1]];
			}
			m_word[cycle.back()] = temp;
		}
	}

	unsigned operator[](unsigned i) const { return m_word[i]; }
};

// 2x2x2
// https://www.desmos.com/calculator/6pqhh5uipy
class Puzzle {
	using Orientation = unsigned;
	using Color = unsigned;

	class Sticker {
		friend Puzzle;
		Orientation m_orientation {};
		Color m_color {};

		Sticker (Orientation o, Color c)
		:	m_orientation{o}, m_color{c} {}

	public:
		auto const& orientation() const { return m_orientation; }
		auto const& color() const { return m_color; }
	};

	std::vector<Sticker> m_stickers {
		{ 0, 0}, { 1, 0}, { 2, 0}, { 3, 0},
		{ 4, 1}, { 5, 1}, { 6, 1}, { 7, 1},
		{ 8, 2}, { 9, 2}, {10, 2}, {11, 2},
		{12, 3}, {13, 3}, {14, 3}, {15, 3},
		{16, 4}, {17, 4}, {18, 4}, {19, 4},
		{20, 5}, {21, 5}, {22, 5}, {23, 5},
	};

	class Move : public Permutation {
		friend Puzzle;
		using Permutation::Permutation;
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
	Puzzle() = default;

	// The only possible way our user can modify this puzzle's state is by
	// picking it up and twisting it :)
	Puzzle& turn(Move const& move) {
		stdr::for_each(m_stickers, [&] (auto& sticker) {
			sticker.m_orientation = move[sticker.m_orientation];
		});
		return *this;
	}

	auto const& stickers() const { return m_stickers; }
	auto const& moves() const { return m_moves; }

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
};
