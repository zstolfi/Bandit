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

/* ~~ Matrix Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

template <class T>
concept IsScalar = std::convertible_to<T, double>; // TODO

enum struct MatrixKind {
	Data = 0, // Property-less 1D or 2D array of scalars.

	// Accessed with [i][j]:
	AnyMatrix = 100, // M x N matrix.
	TransformationMatrix = 101, // N x N matrix.

	// Accessed with X, Y, Z, W:
	AnyVector = 200, // M x 1 matrix.
	NormalVector = 201, // M x 1 matrix which always has unit length.

	// Accessed with R, G, B, A
	ColorVector = 202, // M x 1 matrix with values always clamped to [0, 1].
};

template <
	unsigned DimensionM_m,
	unsigned DimensionN_m,
	MatrixKind Kind_m=MatrixKind::AnyMatrix,
	IsScalar Scalar_t=double
>
class Matrix {
	std::array<Scalar_t, DimensionM_m * DimensionN_m> elements_m {};

	template <MatrixKind K>
	using Relative = Matrix<DimensionM_m, DimensionN_m, K, Scalar_t>;
	using enum MatrixKind;

	// TODO: Figure out how to friend many Matrix types at once.
	friend Relative<Data>;
	friend Relative<AnyMatrix>;
	friend Relative<TransformationMatrix>;
	friend Relative<AnyVector>;
	friend Relative<NormalVector>;
	friend Relative<ColorVector>;

public:
// Compile-time properties
// -----------------------
	using ScalarType = Scalar_t;
	using DataType = Relative<Data>;

	// Every instantiation has exactly one of these three to be true.
	bool static constexpr IsData   {unsigned(Kind_m) / 100 == 0};
	bool static constexpr IsMatrix {unsigned(Kind_m) / 100 == 1};
	bool static constexpr IsVector {unsigned(Kind_m) / 100 == 2};

	// Matrix kinds
	bool static constexpr IsJustMatrix {Kind_m == AnyMatrix};
	bool static constexpr IsTransformation {Kind_m == TransformationMatrix};

	// Vector kinds
	bool static constexpr IsJustVector {Kind_m == AnyVector};
	bool static constexpr IsNormal {Kind_m == NormalVector};
	bool static constexpr IsColor {Kind_m == ColorVector};

	// Compile-time getters
	auto static constexpr Dimension() requires IsMatrix
	{ return std::pair {DimensionM_m, DimensionN_m}; }

	unsigned static constexpr Dimension() requires IsVector
	{ return DimensionM_m; }

	MatrixKind static constexpr Kind() { return Kind_m; }

public:
	// Further public properties
	bool static constexpr UseInRegularParameters {true};
	auto operator<=>(Matrix const&) const = default;

// Constructors
// ------------
	Matrix() requires (IsData) = default;

	Matrix() requires (IsJustMatrix || IsVector)
	{ fill(0); normalize(); }

	Matrix() requires (IsMatrix && !IsJustMatrix)
	{ fill([] (auto i, auto j) { return i==j? 1: 0; }); }

	Matrix(std::convertible_to<Scalar_t> auto const& ... elements)
//	requires (sizeof ... (elements) == elements_m.size())
	:	elements_m {Scalar_t(elements) ... } { normalize(); }

	Matrix(std::initializer_list<Scalar_t> il)
//	requires (il.size() == elements_m.size())
	{ stdr::copy(il, stdr::begin(elements_m)); normalize(); }

	Matrix(Relative<Data> const& data)
	:	elements_m {data.elements_m} { normalize(); }

// Data Elements
// -------------
	// For use with low-level API's, raw access is always allowed.
	[[nodiscard]]
	auto const* pointer() const { return elements_m.data(); }

	// Only data matrices allow the subscript operator. For now, this makes
	// 4-dimensional vectors the upper bound.
	[[nodiscard]]
	Scalar_t const& operator[](unsigned i) const requires (IsData)
	{ return elements_m[i]; }

	[[nodiscard]]
	Scalar_t const& operator[](unsigned i, unsigned j) const requires (IsData)
	{ return elements_m[i * Dimension().first + j]; }

	// Only non-const Data is allowed to be modified directly, un-normalized.
	Scalar_t& operator[](unsigned i) requires (IsData)
	{ return elements_m[i]; }

	[[nodiscard]]
	Scalar_t& operator[](unsigned i, unsigned j) requires (IsData)
	{ return elements_m[i * Dimension().first + j]; }

// Vector-only accessors
// ---------------------
#	define DefineElementProperty(Name, Index, Condition)                       \
 	[[nodiscard]] Scalar_t const& Name() const requires(Condition)             \
 	{ return elements_m[Index]; }                                              \
 	                                                                           \
 	void Name(Scalar_t const& value) requires(Condition)                       \
 	{ elements_m[Index] = value; normalize(); }                                \
 	                                                                           \
 	template <std::invocable<Scalar_t> Fn>                                     \
 	void Name(Fn&& f) requires(Condition)                                      \
 	{ elements_m[Index] = f(elements_m[Index]); normalize(); }

	// Vector/Normal
	DefineElementProperty(x, 0, (IsJustVector || IsNormal) && Dimension() > 0);
	DefineElementProperty(y, 1, (IsJustVector || IsNormal) && Dimension() > 1);
	DefineElementProperty(z, 2, (IsJustVector || IsNormal) && Dimension() > 2);
	DefineElementProperty(w, 3, (IsJustVector || IsNormal) && Dimension() > 3);

	// Color
	DefineElementProperty(r, 0, IsColor && Dimension() > 0);
	DefineElementProperty(g, 1, IsColor && Dimension() > 1);
	DefineElementProperty(b, 2, IsColor && Dimension() > 2);
	DefineElementProperty(a, 3, IsColor && Dimension() > 3);

#	undef DefineElementProperty

// Conversions
// -----------
	// The user is allowed to peek into the underlying data of any Vector.
	Relative<Data> const& data() const requires (!IsData)
	{ return (Relative<Data> const&)*this; }

	// Vectors are allowed to downcast to Normals
	operator Relative<NormalVector>() const requires (IsJustVector)
	{ return Relative<NormalVector> {data()}; }

	// ... and Normals are allowed to upcast to Vectors.
	operator Relative<AnyVector>() const requires (IsNormal)
	{ return Relative<AnyVector> {data()}; }

// Operators
// ---------
	Matrix operator+() const requires (!IsData) {
		Matrix result {*this};
		for (Scalar_t& e: result.elements_m) e = +e;
		result.normalize();
		return result;
	}

	Matrix operator-() const requires (!IsData) {
		Matrix result {*this};
		for (Scalar_t& e: result.elements_m) e = -e;
		result.normalize();
		return result;
	}

	Matrix const& operator+=(Matrix const& other) requires (!IsData) {
		for (auto&& [left, right]: stdv::zip(elements_m, other.elements_m)) {
			left += right;
		}
		normalize();
		return *this;
	}

	Matrix const& operator-=(Matrix const& other) requires (!IsData) {
		for (auto&& [left, right]: stdv::zip(elements_m, other.elements_m)) {
			left -= right;
		}
		normalize();
		return *this;
	}

	Matrix const& operator*=(Scalar_t const& value) requires (!IsData) {
		for (Scalar_t& e: elements_m) e *= value;
		normalize();
		return *this;
	}

	Matrix const& operator/=(Scalar_t const& value) requires (!IsData) {
		if (value != 0) {
			for (Scalar_t& e: elements_m) e /= value;
		}
		normalize();
		return *this;
	}

	friend Matrix operator+(Matrix left, Matrix const& right)
	requires (!IsData) { return left += right; }

	friend Matrix operator-(Matrix left, Matrix const& right)
	requires (!IsData) { return left -= right; }

	friend Matrix operator*(Matrix left, Scalar_t const& right)
	requires (!IsData) { return left *= right; }

	friend Matrix operator*(Scalar_t const& left, Matrix right)
	requires (!IsData) { return right *= left; }

	friend Matrix operator/(Matrix left, Scalar_t const& right)
	requires (!IsData) { return left /= right; }

// Vector Properties
// -----------------
	[[nodiscard]]
	Scalar_t length() const requires (IsVector) {
		if constexpr(IsNormal) return 1;
		return std::sqrt(length2());
	}

	void length(Scalar_t const& value) requires (IsVector) {
		if constexpr(IsNormal) return;
		if (length() != 0) {
			Scalar_t ratio {value / length()};
			for (Scalar_t& e: elements_m) e *= ratio;
		}
		else fill([&] (auto i) { return i==0? value: 0; });
	}

	void length(BecomesMultiplied<Scalar_t> m) requires (IsJustVector) {
		for (Scalar_t& e: elements_m) e *= m.operand;
		normalize();
	}

	void length(BecomesDivided<Scalar_t> d) requires (IsJustVector) {
		if (d.operand != 0) {
			for (Scalar_t& e: elements_m) e /= d.operand;
			normalize();
		}
	}

	template <std::invocable<Scalar_t> Fn>
	void length(Fn&& f) requires (IsVector)
	{ length(f(length())); }

	[[nodiscard]]
	Scalar_t length2() const requires (IsVector) {
		if constexpr(IsNormal) return 1;
		return (*this, *this).dot();
	}

	void length2(Scalar_t const& value) requires (IsVector) {
		if constexpr(IsNormal) return;
		length(std::sqrt(value));
		normalize();
	}

	template <std::invocable<Scalar_t> Fn>
	void length2(Fn&& f) requires (IsVector)
	{ length2(f(length2())); }

	[[nodiscard]]
	Relative<NormalVector> direction() const requires (IsJustVector)
	{ return Relative<NormalVector> {*this}; }

	void direction(Relative<NormalVector> normal) requires (IsJustVector) {
		Scalar_t len = length();
		for (Scalar_t& e: normal.elements_m) e *= len;
		*this = normal;
		normalize();
	}

	template <std::invocable<Relative<NormalVector>> Fn>
	void direction(Fn&& f) requires (IsJustVector)
	{ direction(f(direction())); }

// Matrix Properties
// -----------------
	// TODO

private:
	void fill(Scalar_t value) {
		for (Scalar_t& e: elements_m) e = value;
	}

	template <std::invocable<unsigned> Fn>
	void fill(Fn&& f) {
		for (unsigned i=0; i<elements_m.size(); i++) {
			elements_m[i] = f(i);
		}
	}

	template <std::invocable<unsigned, unsigned> Fn>
	void fill(Fn&& f) {
		for (unsigned i=0; i<Dimension().first; i++)
		for (unsigned j=0; j<Dimension().second; j++) {
			elements_m[i * Dimension().first + j] = f(i, j);
		}
	}

	void normalize() {/* Do nothing by default. */}

	void normalize() requires (IsNormal) {
		Scalar_t len2 = As<AnyVector>().length2();
		if (len2 != 0) {
			for (Scalar_t& e: elements_m) e /= std::sqrt(len2);
		}
		else fill([] (auto i) { return i==0? 1: 0; });
	}

	void normalize() requires (IsColor) {
		for (Scalar_t& e: elements_m) {
			if (e < 0) e = 0;
			if (e > 1) e = 1;
		}
	}

	template <MatrixKind K>
	Relative<K> As()
	{ return (Relative<K>&)*this; }
};

// TODO: Make Vector alias only instantiable with VectorKinds.
template <unsigned Dimension, MatrixKind Kind, IsScalar Scalar=double>
using Vector = Matrix<Dimension, 1, Kind, Scalar>;

using Matx4 = Matrix<4, 4, MatrixKind::AnyMatrix>;
using Coord3 = Vector<3, MatrixKind::AnyVector>;
using Norm3 = Vector<3, MatrixKind::NormalVector>;
using Color3 = Vector<3, MatrixKind::ColorVector>;

/* ~~ Compound Types ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

template <
	unsigned Dimension_m,
	IsScalar Scalar_t=double
>
class Ray {
	using enum MatrixKind;
	using CospacialVector = Vector<Dimension_m, AnyVector, Scalar_t>;
	using CospacialNormal = Vector<Dimension_m, NormalVector, Scalar_t>;

	CospacialVector position_m {};
	CospacialNormal direction_m {};

public:
	using ScalarType = Scalar_t;
	unsigned static constexpr Dimension() { return Dimension_m; }

// Constructors
// ------------
	Ray() = default;

	Ray(CospacialVector const& position, CospacialNormal const& direction)
	:	position_m {position}
	,	direction_m {direction} {}


// Properties
// ----------
	// TODO
};

template <
	unsigned Dimension_m,
	IsScalar Scalar_t=double
>
class Plane {
	using enum MatrixKind;
	using CospacialRay = Ray<Dimension_m, Scalar_t>;
	using CospacialVector = Vector<Dimension_m, AnyVector, Scalar_t>;
	using CospacialNormal = Vector<Dimension_m, NormalVector, Scalar_t>;

	Scalar_t length_m {};
	CospacialNormal direction_m {};

public:
	using ScalarType = Scalar_t;
	unsigned static constexpr Dimension() { return Dimension_m; }

// Constructors
// ------------
	Plane() = default;

	Plane(Scalar_t distance, CospacialNormal const& direction)
	:	length_m {distance}
	,	direction_m {direction} {}

	Plane(CospacialVector const& origin, CospacialNormal const& direction)
	:	length_m {(direction, origin).dot()}
	,	direction_m {direction} {}

	Plane(CospacialRay const& ray)
	:	Plane{ray.origin(), ray.direction()} {}

	Plane(std::convertible_to<CospacialVector> auto const& ... points)
	requires (sizeof ... (points) == Dimension_m) {/* TODO */}

// Properties
// ----------
	[[nodiscard]]
	Scalar_t const& length() const
	{ return length_m; }

	void length(Scalar_t const& value)
	{ length_m = value; }

	template <std::invocable<Scalar_t> Fn>
	void length(Fn&& f)
	{ length(f(length())); }

	[[nodiscard]]
	CospacialNormal const& direction() const
	{ return direction_m; }

	void direction(CospacialNormal const& value)
	{ direction_m = value; }

	template <std::invocable<CospacialNormal> Fn>
	void direction(Fn&& f)
	{ direction(f(direction())); }

	[[nodiscard]]
	CospacialVector origin() const
	{ return length() * direction(); }

	void origin(CospacialVector const& value)
	{ length_m = (direction_m, value).dot(); }

	template <std::invocable<CospacialVector> Fn>
	void origin(Fn&& f)
	{ origin(f(origin())); }

};

using Ray3 = Ray<3>;
using Plane3 = Plane<3>;

/* ~~ Relational Properties ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// Regular parameters enable the syntax style:
//	Location home, work;
//	std::cout << home.address();
//	std::cout << work.address();
//	std::cout << (home, work).distance();

template <class ... Args>
class RegularParameters: std::tuple<Args ... > {
	using Tuple = std::tuple<Args ... >;
	using enum MatrixKind;

	template <unsigned I>
	using Get = std::tuple_element_t<I, Tuple>;

	// Determine What types of argument tuples to support properties for.
	template <MatrixKind ... Ks>
	bool static constexpr IsVectorPairOf {
		// We have two arguments, both of which are Vectors.
		sizeof ... (Args) == 2 &&
		Get<0>::IsVector && Get<1>::IsVector &&
		// Any combination of VectorKinds provided are allowed.
		((Get<0>::Kind() == Ks) || ... ) &&
		((Get<1>::Kind() == Ks) || ... )
	};


public:
	RegularParameters(Args const& ... args): Tuple {args ... } {}

	auto dot() const requires IsVectorPairOf<AnyVector, NormalVector> {
		// TODO: Make Get<0>ScalarType a private type alias, maybe set it to
		// void if our arguments are not vectors.
		typename Get<0>::ScalarType result {};
		for (unsigned i=0; i<Get<0>::Dimension(); i++) {
			result += get<0>().data()[i] * get<1>().data()[i];
		}
		return result;
	}

	auto distance() const requires IsVectorPairOf<AnyVector, NormalVector> {
		return std::sqrt(distance2());
	}

	auto distance2() const requires IsVectorPairOf<AnyVector, NormalVector> {
		return (get<0>() - get<1>()).length2();
	}

private:
	// Member access helper
	template <unsigned I>
	auto const& get() const { return std::get<I>(*this); }
};

template <class T>
concept IsRpOverloadable =
	requires { T::UseInRegularParameters; } &&
	T::UseInRegularParameters == true
;

template <IsRpOverloadable T, IsRpOverloadable U>
auto operator,(T const& a, U const& b) { return RegularParameters {a, b}; }

/* ~~ Permutation Class ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

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
