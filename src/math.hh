#pragma once
#include "util.hh"

double constexpr TWO_PI = 6.2831853071795864769;

template <class T>
struct BecomesAdded {
	T operand {0};
	auto operator()(auto value) { return value + operand; }
};

template <class T>
struct BecomesSubtracted {
	T operand {0};
	auto operator()(auto value) { return value - operand; }
};

template <class T>
struct BecomesMultiplied {
	T operand {1};
	auto operator()(auto value) { return value * operand; }
};

template <class T>
struct BecomesDivided {
	T operand {1};
	auto operator()(auto value) { return value / operand; }
};

struct Becomes_Arg {} static constexpr Becomes {};

template <class T>
auto operator+=(Becomes_Arg, T x) { return BecomesAdded {x}; }

template <class T>
auto operator-=(Becomes_Arg, T x) { return BecomesSubtracted {x}; }

template <class T>
auto operator*=(Becomes_Arg, T x) { return BecomesMultiplied {x}; }

template <class T>
auto operator/=(Becomes_Arg, T x) { return BecomesDivided {x}; }

/* ~~ Linear Algebra ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
// for 3D graphics

template <class T>
concept IsScalar = std::convertible_to<T, double>; // TODO

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

	// Keep track of internal properties here.
	using enum VectorKind;
	bool static constexpr IsData {Kind == Data};
	bool static constexpr IsCoordinate {Kind == Coordinate};
	bool static constexpr IsNormal {Kind == Normal};
	bool static constexpr IsColor {Kind == Color};

	template <VectorKind K>
	using Relative = Vector<Dimension, K, Scalar>;

	// TODO: Figure out how to friend many Vector types at once.
	friend Relative<Data>;
	friend Relative<Coordinate>;
	friend Relative<Normal>;
	friend Relative<Color>;

public:
	// Further public properties
	using DataType = Relative<Data>;
	bool static constexpr UseInRegularParameters {true};

	auto operator<=>(Vector const&) const = default;

	Vector() { fillConstant(0); normalize(); }

	Vector(std::convertible_to<Scalar> auto const& ... elements)
	:	elements_m {Scalar(elements) ... } { normalize(); }

	Vector(std::initializer_list<Scalar> il)
	{ stdr::copy(il, stdr::begin(elements_m)); normalize(); }

	Vector(Relative<Data> const& data)
	:	elements_m {data.elements_m} { normalize(); }

	// Data elements //
	[[nodiscard]]
	Scalar const& operator[](unsigned i) const requires (IsData)
	{ return elements_m[i]; }

	// No need to normalize, non-Data Vectors only have const access.
	Scalar& operator[](unsigned i) requires (IsData)
	{ return elements_m[i]; }

	// Coordinate/Color elements //
#	define DefineElementProperty(Name, Index, Condition)                       \
 	[[nodiscard]] Scalar const& Name() const requires(Condition)               \
 	{ return elements_m[Index]; }                                              \
 	                                                                           \
 	void Name(Scalar const& value) requires(Condition)                         \
 	{ elements_m[Index] = value; normalize(); }                                \
 	                                                                           \
 	template <std::invocable<Scalar> Fn>                                       \
 	void Name(Fn&& f) requires(Condition)                                      \
 	{ elements_m[Index] = f(elements_m[Index]); normalize(); }

	// Coordinate/Normal
	DefineElementProperty(x, 0, (IsCoordinate || IsNormal) && Dimension > 0);
	DefineElementProperty(y, 1, (IsCoordinate || IsNormal) && Dimension > 1);
	DefineElementProperty(z, 2, (IsCoordinate || IsNormal) && Dimension > 2);
	DefineElementProperty(w, 3, (IsCoordinate || IsNormal) && Dimension > 3);

	// Color
	DefineElementProperty(r, 0, IsColor && Dimension > 0);
	DefineElementProperty(g, 1, IsColor && Dimension > 1);
	DefineElementProperty(b, 2, IsColor && Dimension > 2);
	DefineElementProperty(a, 3, IsColor && Dimension > 3);

#	undef DefineElementProperty

	// Conversions //

	// The user is allowed to peek into the underlying data of any Vector.
	Relative<Data> const& data() const requires (!IsData)
	{ return (Relative<Data> const&)*this; }

	// Coordinates are allowed to downcast to Normals
	operator Relative<Normal>() const requires (IsCoordinate)
	{ return Relative<Normal> {data()}; }

	// ... and Normals are allowed to upcast to Coordinates.
	operator Relative<Coordinate>() const requires (IsNormal)
	{ return Relative<Coordinate> {data()}; }

	// Operators //

	Vector operator+() const requires (!IsData) {
		Vector result {*this};
		for (Scalar& e: result.elements_m) e = +e;
		result.normalize();
		return result;
	}

	Vector operator-() const requires (!IsData) {
		Vector result {*this};
		for (Scalar& e: result.elements_m) e = -e;
		result.normalize();
		return result;
	}

	Vector const& operator+=(Vector const& other) requires (!IsData) {
		for (auto&& [left, right]: stdv::zip(elements_m, other.elements_m)) {
			left += right;
		}
		normalize();
		return *this;
	}

	Vector const& operator-=(Vector const& other) requires (!IsData) {
		for (auto&& [left, right]: stdv::zip(elements_m, other.elements_m)) {
			left -= right;
		}
		normalize();
		return *this;
	}

	Vector const& operator*=(Scalar const& value) requires (!IsData) {
		for (Scalar& e: elements_m) e *= value;
		normalize();
		return *this;
	}

	Vector const& operator/=(Scalar const& value) requires (!IsData) {
		if (value != 0) {
			for (Scalar& e: elements_m) e /= value;
		}
		normalize();
		return *this;
	}

	friend Vector operator+(Vector left, Vector const& right)
	requires (!IsData) { return left += right; }

	friend Vector operator-(Vector left, Vector const& right)
	requires (!IsData) { return left -= right; }

	friend Vector operator*(Vector left, Scalar const& right)
	requires (!IsData) { return left *= right; }

	friend Vector operator*(Scalar const& left, Vector right)
	requires (!IsData) { return right *= left; }

	friend Vector operator/(Vector left, Scalar const& right)
	requires (!IsData) { return left /= right; }

	// Properties //

	[[nodiscard]]
	Scalar length() const requires (IsCoordinate) {
		return std::sqrt(length2());
	}

	void length(Scalar const& value) requires (IsCoordinate) {
		if (length() != 0) {
			Scalar ratio {value / length()};
			for (Scalar& e: elements_m) e *= ratio;
		}
		else fill([&] (unsigned i) { return i==0? value: 0; });
	}

	void length(BecomesMultiplied<Scalar> m) requires (IsCoordinate) {
		for (Scalar& e: elements_m) e *= m.operand;
	}

	void length(BecomesDivided<Scalar> d) requires (IsCoordinate) {
		if (d.operand != 0) {
			for (Scalar& e: elements_m) e /= d.operand;
		}
	}

	template <std::invocable<Scalar> Fn>
	void length(Fn&& f) requires (IsCoordinate)
	{ length(f(length())); }

	[[nodiscard]]
	Scalar length() const requires (IsNormal) {
		return 1;
	}

	[[nodiscard]]
	Scalar length2() const requires (IsCoordinate) {
		Scalar result {0};
		for (Scalar const& e: elements_m) result += e * e;
		return result;
	}

	void length2(Scalar const& value) requires (IsCoordinate) {
		length(std::sqrt(value));
	}

	template <std::invocable<Scalar> Fn>
	void length2(Fn&& f) requires (IsCoordinate) {
		length2(f(length2()));
	}

	[[nodiscard]]
	Scalar length2() const requires (IsNormal) {
		return 1;
	}

	[[nodiscard]]
	Relative<Normal> direction() const requires (IsCoordinate) {
		return Relative<Normal> {*this};
	}

	void direction(Relative<Normal> normal) requires (IsCoordinate) {
		Scalar len = length();
		for (Scalar& e: normal.elements_m) e *= len;
		*this = normal;
	}

	template <std::invocable<Relative<Normal>> Fn>
	void direction(Fn&& f) requires (IsCoordinate) {
		direction(f(direction()));
	}

private:
	void fillConstant(Scalar value) {
		for (Scalar& e: elements_m) e = value;
	}

	void fill(auto&& f) {
		for (unsigned i=0; Scalar& e: elements_m) e = f(i++);
	}

	void normalize() {/* Do nothing. */}

	void normalize() requires (IsNormal) {
		Scalar len2 = As<Coordinate>().length2();
		if (len2 != 0) {
			for (Scalar& e: elements_m) e /= std::sqrt(len2);
		}
		else fill([] (unsigned i) { return i==0? 1: 0; });
	}

	void normalize() requires (IsColor) {
		for (Scalar& e: elements_m) {
			if (e < 0) e = 0;
			if (e > 1) e = 1;
		}
	}

	template <VectorKind K>
	Relative<K> As()
	{ return (Relative<K>&)*this; }

	template <VectorKind K>
	Relative<K> As() const
	{ return (Relative<K> const&)*this; }
};

using Coord3 = Vector<3, VectorKind::Coordinate>;
using Norm3 = Vector<3, VectorKind::Normal>;
using Color3 = Vector<3, VectorKind::Color>;

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
	using Tuple = std::tuple<Args ... >;
	using enum VectorKind;

	template <unsigned I>
	using Get = std::tuple_element_t<I, Tuple>;

	// Determine What types of argument tuples to support properties for.
	template <VectorKind ... Ks>
	bool static constexpr VectorPair {
		sizeof ... (Args) == 2 &&
		Get<0>::Dimension == Get<1>::Dimension &&
		Get<0>::Kind == Get<1>::Kind &&
		((Get<0>::Kind == Ks) || ... )
	};


public:
	RegularParameters(Args const& ... args): Tuple {args ... } {}

	auto dot() const requires VectorPair<Coordinate> {
		typename Get<0>::Scalar result {};
		/* ... */
		return result;
	}

	auto distance() const requires VectorPair<Coordinate> {
		typename Get<0>::Scalar result {};
		/* ... */
		return result;
	}

	auto distance2() const requires VectorPair<Coordinate> {
		typename Get<0>::Scalar result {};
		/* ... */
		return result;
	}

private:
	// Member access helper.
	template <unsigned I>
	auto const& get() const { return std::get<I>(*this); }
};

template <class T>
concept RegularParametersTarget =
	requires { T::UseInRegularParameters; } &&
	T::UseInRegularParameters == true
;

template <RegularParametersTarget T, RegularParametersTarget U>
auto operator,(T const& a, U const& b) { return RegularParameters {a, b}; }

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

std::ostream& operator<<(std::ostream& os, Permutation const& p) {
	return os << std::format("{}", p);
}
