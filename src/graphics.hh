#pragma once
#include <glad/glad.h> // TODO: Use non-GL types.
#include "puzzle.hh"
#include "util.hh"

auto const Colors = std::vector<std::array<float, 3>> {
	{{0.9, 0.5, 0.2}}, // Orange
	{{0.9, 0.2, 0.2}}, // Red
	{{0.9, 0.9, 0.9}}, // White
	{{0.9, 0.9, 0.2}}, // Yellow
	{{0.2, 0.3, 0.9}}, // Blue
	{{0.1, 0.9, 0.2}}, // Green
};

auto const ColorsBackground = std::vector<std::array<float, 3>> {
	{{0.7, 0.7, 0.7}}, // Unsolved
	{{0.6, 0.7, 0.6}}, // Solved
};

template <IsPuzzle Puzzle>
class Model {
	std::vector<GLfloat> vertices_m {};
	std::vector<GLuint> indices_m {};
	std::map<unsigned, std::vector<GLuint>> stickerIndices_m {};

public:
	Model(Puzzle puzzle={}) {
		GLuint index = 0;
		for (auto const& sticker: puzzle.appearance().stickers) {
			auto const& polygon = sticker.polygon();
			auto const& color = Colors[sticker.color()];
			for (Coord3 const& c: polygon) {
				stickerIndices_m[sticker.position()].push_back(index);
				vertices_m.push_back(c.x());
				vertices_m.push_back(c.y());
				vertices_m.push_back(c.z());
				vertices_m.push_back(color[0]);
				vertices_m.push_back(color[1]);
				vertices_m.push_back(color[2]);
				index++;
			}
			indices_m.push_back(index-4 + 0);
			indices_m.push_back(index-4 + 1);
			indices_m.push_back(index-4 + 2);
			indices_m.push_back(index-4 + 2);
			indices_m.push_back(index-4 + 3);
			indices_m.push_back(index-4 + 0);
		}
	}

	void update(Puzzle const& puzzle) {
		for (auto const& sticker: puzzle.appearance().stickers) {
			auto color = Colors[sticker.color()];
			auto position = sticker.position();
			for (unsigned index: stickerIndices_m[position]) {
				vertices_m[6 * index + 3] = color[0];
				vertices_m[6 * index + 4] = color[1];
				vertices_m[6 * index + 5] = color[2];
			}
		}
	}

	auto const& vertices() const { return vertices_m; }
	auto const& indices() const { return indices_m; }
};
