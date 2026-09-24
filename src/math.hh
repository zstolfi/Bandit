#pragma once
#include "util.hh"

class Permutation {
	using Index = unsigned;
	using Cycle = std::vector<Index>;
	std::vector<Cycle> cycles_m {};

public:
	Permutation() = default;
	auto operator<=>(Permutation const&) const = default;

	Permutation(std::vector<Cycle> const& cycles) {
		// Take notes on user input.
		std::map<Index, unsigned> counts {};
		for (auto const& cycle: cycles) {
			for (Index i: cycle) {
				counts[i]++;
			}
		}
		// Elements cannot be repeated within or between cycles
		for (unsigned count: counts | stdv::values) if (count != 1) {
			throw std::invalid_argument {"invalid cycle notation"};
		}
		// Assign.
		cycles_m = cycles;
		normalize();
	}

	// TODO: Either remove or replace with "word()" function.
	Index operator[](Index i) const {
		if (i > maxIndex()) return i;
		for (auto const& cycle: cycles_m) {
			for (unsigned j=0; j<cycle.size(); j++) {
				if (cycle[j] == i) {
					return (j+1 < cycle.size())
					?	cycle[j+1]
					:	cycle[0];
				}
			}
		}
		return i;
	}

	// Permute any range in-place.
	template <stdr::random_access_range Range>
	void shuffle(Range& input) const {
		if (stdr::size(input) < maxIndex()) {
			throw std::invalid_argument {"invalidly sized input range"};
		}
		for (auto const& cycle: cycles_m) {
			for (auto [i, j]: cycle | stdv::pairwise) {
				std::swap(input[i], input[j]);
			}
		}
	}

private:
	Index maxIndex() const { // MUST NOT be called pre-normalization.
		if (cycles_m.empty()) return 0;
		return cycles_m.back()[0];
	}

	void normalize() {
		for (auto& cycle: cycles_m) {
			stdr::rotate(cycle, stdr::max_element(cycle));
		}
		stdr::sort(cycles_m, stdr::less {},
			[] (auto const& cycle) { return cycle[0]; }
		);
		std::erase_if(cycles_m,
			[] (auto const& cycle) { return cycle.size() < 2; }
		);
	}
};
