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
		if (index > maxIndex()) return index;
		for (auto const& cycle : m_cycles) {
			for (unsigned i=0; i<cycle.size(); i++) {
				if (cycle[i] == index) {
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
		if (stdr::size(input) < maxIndex()) {
			throw std::invalid_argument {"input range cannot be permuted"};
		}
		for (auto const& cycle : m_cycles) {
			for (auto [i, j] : cycle | stdv::pairwise) {
				std::swap(input[i], input[j]);
			}
		}
	}

private:
	unsigned maxIndex() const { // Must NOT be called pre-normalization.
		return m_cycles.back()[0];
	}

	void normalize() {
		for (auto& cycle : m_cycles) {
			stdr::rotate(cycle, stdr::max_element(cycle));
		}
		stdr::sort(m_cycles, stdr::less {},
			[] (auto const& cycle) { return cycle[0]; }
		);
		std::erase_if(m_cycles,
			[] (Cycle const& c) { return c.size() < 2; }
		);
	}
};
