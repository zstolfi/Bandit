#pragma once
#include "util.hh"

// 2x2x2
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
		auto operator<=>(Sticker const&) const = default;
		auto const& orientation() const { return m_orientation; }
		auto const& color()       const { return m_color;       }
	};

	std::set<Sticker> m_stickers {
		{ 0, 0}, { 1, 0}, { 2, 0}, { 3, 0},
		{ 4, 1}, { 5, 1}, { 6, 1}, { 7, 1},
		{ 8, 2}, { 9, 2}, {10, 2}, {11, 2},
		{12, 3}, {13, 3}, {14, 3}, {15, 3},
		{16, 4}, {17, 4}, {18, 4}, {19, 4},
		{20, 5}, {21, 5}, {22, 5}, {23, 5},
	};

public:
	Puzzle() = default;

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
};
