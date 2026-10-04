#pragma once
#include "util.hh"

double constexpr TWO_PI = 6.2831853071795864769;

struct BecomesAdded {
	double operand {0};
	double operator()(double value) { return value + operand; }
};

struct BecomesSubtracted {
	double operand {0};
	double operator()(double value) { return value - operand; }
};

struct BecomesMultiplied {
	double operand {1};
	double operator()(double value) { return value * operand; }
};

struct BecomesDivided {
	double operand {1};
	double operator()(double value) { return value / operand; }
};

struct Becomes_Arg {} static constexpr Becomes {};
auto operator+=(Becomes_Arg, double n) { return BecomesAdded {n}; }
auto operator-=(Becomes_Arg, double n) { return BecomesSubtracted {n}; }
auto operator*=(Becomes_Arg, double n) { return BecomesMultiplied {n}; }
auto operator/=(Becomes_Arg, double n) { return BecomesDivided {n}; }

/* ~~ Linear Algebra ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
// for 3D graphics

template <class>
concept IsScalar = true; // TODO

enum struct VectorKind {
	Data, // [0] [1] [2] ...
	Coordinate, Normal, // X, Y, Z, W
	Color, // R, G, B, A
};

template <
	unsigned Dimension_p,
	VectorKind Kind_p,
	IsScalar Scalar_p=double
>
class Vector {
public:
	// Our template parameters are no secret.
	unsigned static constexpr Dimension	{Dimension_p};
	VectorKind static constexpr Kind {Kind_p};
	using Scalar = Scalar_p;

private:
	std::array<Scalar, Dimension> elements_m {};

	using enum VectorKind;
	bool static constexpr IsData {Kind == Data};
	bool static constexpr IsCoordinate {Kind == Coordinate || Kind == Normal};
	bool static constexpr IsColor {Kind == Color};
	using UnaryFn = std::function<Scalar (Scalar)>;

public:
	auto operator<=>(Vector const&) const = default;

	Vector() { stdr::fill(elements_m, 0); }

	Vector(auto const& ... elements): elements_m{Scalar(elements) ... } {}

	// Data elements
	[[nodiscard]]
	Scalar const& operator[](unsigned i) const requires (IsData)
	{ return elements_m[i]; }

	Scalar& operator[](unsigned i) requires (IsData)
	{ return elements_m[i]; }

	[[nodiscard]]
	Scalar const& x() const requires (IsCoordinate && Dimension >= 1)
	{ return elements_m[0]; }

	// Coordinate elements
	void x(Scalar const& value) requires (IsCoordinate && Dimension >= 1)
	{ elements_m[0] = value; }

	void x(UnaryFn f) requires (IsCoordinate && Dimension >= 1)
	{ elements_m[0] = f(elements_m[0]); }

	[[nodiscard]]
	Scalar const& y() const requires (IsCoordinate && Dimension >= 2)
	{ return elements_m[1]; }

	void y(Scalar const& value) requires (IsCoordinate && Dimension >= 2)
	{ elements_m[1] = value; }

	void y(UnaryFn f) requires (IsCoordinate && Dimension >= 2)
	{ elements_m[1] = f(elements_m[1]); }

	[[nodiscard]]
	Scalar const& z() const requires (IsCoordinate && Dimension >= 3)
	{ return elements_m[2]; }

	void z(Scalar const& value) requires (IsCoordinate && Dimension >= 3)
	{ elements_m[2] = value; }

	void z(UnaryFn f) requires (IsCoordinate && Dimension >= 3)
	{ elements_m[2] = f(elements_m[2]); }

	// Properties
	[[nodiscard]]
	Scalar length() const requires (IsCoordinate) {
		Scalar result {0};
		for (unsigned i=0; i<Dimension; i++) {
			result += elements_m[i] * elements_m[i];
		}
		result = std::sqrt(result);
		return result;
	}

	void length(Scalar const& value) requires (IsCoordinate) {
		Scalar ratio {value / length()};
		for (unsigned i=0; i<Dimension; i++) {
			elements_m[i] *= ratio;
		}
	}

	void length(BecomesMultiplied m) requires (IsCoordinate) {
		std::print("Clever square root avoidance!\n");
		for (unsigned i=0; i<Dimension; i++) {
			elements_m[i] *= m.operand;
		}
	}

	void length(UnaryFn f) requires (IsCoordinate)
	{ length(f(length())); }
};

using Coord3 = Vector<3, VectorKind::Coordinate>;
using Norm3 = Vector<3, VectorKind::Normal>;
using Color = Vector<3, VectorKind::Color>;

/* ~~ Matrices ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

enum struct MatrixKind {
	Data,
	Transformation, Rotation
};

template <
	unsigned DimensionI,
	unsigned DimensionJ,
	MatrixKind Kind,
	IsScalar Scalar=double
>
class Matrix {
	std::array<Scalar, DimensionI * DimensionJ> elements_m;

public:
	Matrix();
	auto operator<=>(Matrix const&) const = default;

	/* ... */
};

/* ~~ Relational Properties ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// Regular parameters enable the syntax style:
//	Location home, work;
//	std::cout << home.address();
//	std::cout << work.address();
//	std::cout << (home, work).distance();

template <class ... Args>
class RegularParameters: std::tuple<Args ... > {
	template <class ... Ts>
	bool static constexpr Arguments =
		std::same_as<std::tuple<Ts ... >, std::tuple<Args ... >>
	;

public:
	RegularParameters(Args const& ... args): std::tuple<Args ... >{args ... } {}

	Coord3::Scalar dot() const requires Arguments<Coord3, Coord3>;
	Coord3::Scalar distance() const requires Arguments<Coord3, Coord3>;
	Coord3::Scalar distance2() const requires Arguments<Coord3, Coord3>;

private:
	template <auto I>
	auto const& get() const { return std::get<I>(*this); }
};

RegularParameters<Coord3, Coord3> operator,(Coord3 a, Coord3 b);

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
	Permutation(Cs const& ... cycles) requires (IsRangeOf<Cs, Index> && ... )
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

std::ostream& operator<<(std::ostream& os, Permutation const& p);
