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
	std::map<unsigned, std::vector<GLuint>> stickerIndices_m {};

public:
	Model(Puzzle puzzle={}) {
		for (auto const& sticker: puzzle.appearance().stickers) {
			auto const& polygon = sticker.polygon();
			auto const& color = Colors[sticker.color()];
			for (unsigned i=0; i<3*polygon.size()+6; i++) {
				GLuint index = vertices_m.size();
				stickerIndices_m[sticker.position()].push_back(index + 6*i);
			}
			addPolygon(polygon, color);
		}
	}

	void update(Puzzle const& puzzle) {
		for (auto const& sticker: puzzle.appearance().stickers) {
			auto color = Colors[sticker.color()];
			auto position = sticker.position();
			for (GLuint index: stickerIndices_m[position]) {
				vertices_m[index + 3] = color[0];
				vertices_m[index + 4] = color[1];
				vertices_m[index + 5] = color[2];
			}
		}
	}

	auto const& vertices() const { return vertices_m; }

private:
	void addPolygon(auto const& polygon, auto const& color) {
		if (polygon.size() < 3) return;
		Coord3 const& p0 = polygon[0];
		for (auto const& [p1, p2]: polygon | stdv::drop(1) | stdv::pairwise) {
			addVertex(p0, color);
			addVertex(p1, color);
			addVertex(p2, color);
		}
	}

	void addVertex(auto const& vertex, auto const& color) {
		vertices_m.push_back(vertex.x());
		vertices_m.push_back(vertex.y());
		vertices_m.push_back(vertex.z());
		vertices_m.push_back(color[0]);
		vertices_m.push_back(color[1]);
		vertices_m.push_back(color[2]);
	}
};
