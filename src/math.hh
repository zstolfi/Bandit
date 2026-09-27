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
	using Index_t = unsigned;
	using Cycle_t = std::vector<Index_t>;
	std::vector<Cycle_t> cycles_m {};

public:
	using Index = Index_t;
	using Cycle = Cycle_t;
	auto operator<=>(Permutation const&) const = default;

	Permutation() = default;

	// Generate at compile time.
	template <class ... Is>
	Permutation(std::initializer_list<Is> ... ils)
	:	Permutation {std::array {Cycle {ils.begin(), ils.end()} ... }} {}

	template <class ... Cs>
	Permutation(Cs const& ... cycles)
	:	Permutation {std::array {cycles} ... } {}

	// Generate at run time.
	Permutation(stdr::input_range auto cycles)
	requires stdr::input_range<stdr::range_value_t<decltype(cycles)>> {
		if (duplicateIndices(cycles)) {
			throw std::invalid_argument {"duplicate cycle indices"};
		}
		stdr::copy(cycles, std::back_inserter(cycles_m));
		normalize();
	}

	// TODO: Add multiplication operator overloads.

	template <stdr::random_access_range Range>
	auto apply(Range const& input) const {
		auto result = input;
		apply_inplace(result);
		return result;
	}

	template <stdr::random_access_range Range>
	auto apply_inplace(Range& result) const {
		if (stdr::size(result) < size()) {
			throw std::invalid_argument {"invalidly sized input"};
		}
		for (auto const& cycle: cycles_m) {
			for (auto [i, j]: cycle | stdv::pairwise) {
				std::swap(result[i], result[j]);
			}
		}
		return result;
	}

	std::size_t size() const {
		if (cycles_m.empty()) return 0;
		return cycles_m.back()[0] + 1;
	}

	auto inverse() const {
		auto result = *this;
		// Inverting is easy. Simply reverse the cycles and voila.
		// Here I keep the first elements in-place to preserve normalization.
		for (auto& cycle: result.cycles_m) {
			stdr::reverse(cycle.begin()+1, cycle.end());
		}
		return result;
	};

	auto cycles() const {
		return cycles_m;
	}

	auto word() const {
		auto result = std::vector<Index> {};
		result.resize(size());
		stdr::iota(result, 0);
		apply_inplace(result);
		return result;
	}

private:
	bool duplicateIndices(stdr::input_range auto cycles) const {
		std::set<Index> indicesUsed {};
		for (auto const& c: cycles)
		for (Index i: c) {
			if (indicesUsed.contains(i)) return true;
			indicesUsed.insert(i);
		}
		return false;
	}

	void normalize() {
		std::erase_if(cycles_m,
			[] (auto const& cycle) { return cycle.size() < 2; }
		);
		for (auto& cycle: cycles_m) {
			stdr::rotate(cycle, stdr::max_element(cycle));
		}
		stdr::sort(cycles_m, stdr::less {},
			[] (auto const& cycle) { return cycle[0]; }
		);
	}
};

/* ~~ Formatting & Printing ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

template <>
struct std::formatter<Permutation>: std::formatter<std::string> {
	auto format(Permutation p, format_context& ctx) const {
		std::string result {};
		auto cycles = p.cycles();
		if (!cycles.empty()) {
			for (auto const& cycle: cycles) {
				result += "(";
				for (bool first=true; auto i: cycle) {
					if (!first) result += ", ";
					result += std::format("{}", i);
					first = false;
				}
				result += ")";
			}
		}
		else result = "()";
		return stdr::copy(result, ctx.out()).out;
	}
};

std::ostream& operator<<(std::ostream& os, Permutation const& p) {
	return os << std::format("{}", p);
}
