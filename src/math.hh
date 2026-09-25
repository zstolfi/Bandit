#pragma once
#include "util.hh"

/* ~~ Linear Algebra ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
// for 3D graphics

class Coord3 {
protected:
	std::array<double, 3> data_m {};

public:
	Coord3(): data_m{0, 0, 0} {}
	Coord3(double x, double y, double z): data_m{x, y, z} {}
	auto operator<=>(Coord3 const&) const = default;

	// Getters    Ex: value = position.x();
	double x() const { return data_m[0]; }
	double y() const { return data_m[1]; }
	double z() const { return data_m[2]; }

	// Setters    Ex: position.x(value);
	// This syntax is strange but it allows us to keep a close eye on our data.
	// If we were to send out references, it means the user could de-normalize
	// our data at will.
	void x(double val) { data_m[0] = val; normalize(); }
	void y(double val) { data_m[1] = val; normalize(); }
	void z(double val) { data_m[2] = val; normalize(); }

	// Properties
	double length2() const {
		return
			data_m[0] * data_m[0] +
			data_m[1] * data_m[1] +
			data_m[2] * data_m[2]
		;
	}

protected:
	virtual void normalize() {/* Do nothing. */}
};

class Norm3: public Coord3 {
public:
	Norm3(): Coord3{1, 0, 0} {}
	Norm3(Coord3 c): Coord3{c} { normalize(); }
	Norm3(double x, double y, double z): Coord3{x, y, z} { normalize(); }
	auto operator<=>(Norm3 const&) const = default;

private:
	void normalize() override {
		bool strict = std::is_constant_evaluated();
		if (length2() != 0.0) {
			double inv = std::pow(length2(), -0.5);
			data_m[0] *= inv;
			data_m[1] *= inv;
			data_m[2] *= inv;
		}
		else {
			// Defining invalid normals at compile time is definitely an error.
			if (strict) throw std::domain_error {"Norm3 division by 0"};
			// Otherwise it's best to just keep our state valid.
			else *this = {};
		}
	}
};

class Ray3 {
	Coord3 origin_m {};
	Norm3 direction_m {};

public:
	Ray3() {}
	Ray3(Coord3 origin, Norm3 direction)
	:	origin_m{origin}
	,	direction_m{direction} {}

	auto operator<=>(Ray3 const&) const = default;

	// Getters
	Coord3 const& origin() const { return origin_m; }
	Norm3 const& direction() const { return direction_m; }

	// Setters
	void origin(Coord3 c) { origin_m = c; }
	void direction(Norm3 n) { direction_m = n; }
};

class Plane3 {
	Coord3 origin_m {};
	Norm3 direction_m {};

public:
	Plane3() {}
	Plane3(Coord3 origin, Norm3 direction)
	:	origin_m{origin}
	,	direction_m{direction} {}
	Plane3(Ray3 const& r)
	:	origin_m{r.origin()}
	,	direction_m{r.direction()} {}

	auto operator<=>(Plane3 const&) const = default;

	// Getters
	Coord3 const& origin() const { return origin_m; }
	Norm3 const& direction() const { return direction_m; }

	// Setters
	void origin(Coord3 c) { origin_m = c; normalize(); }
	void direction(Norm3 n) { direction_m = n; }

//	// Properties
//	double length();

private:
	void normalize() {
		double dist =
			origin_m.x() * direction_m.x() +
			origin_m.y() * direction_m.y() +
			origin_m.z() * direction_m.z()
		;
		origin_m = Coord3 {
			direction_m.x() * dist,
			direction_m.y() * dist,
			direction_m.z() * dist,
		};
	}
};

/* ~~ Group Theory ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
// for puzzle logic

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
