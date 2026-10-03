#pragma once
#include "util.hh"

double constexpr TWO_PI = 6.2831853071795864769;

/* ~~ Linear Algebra ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
// for 3D graphics

class Vect3 {
protected:
	template <class ... >
	friend struct RegularParameters;

	using Scalar = double;
	std::array<Scalar, 3> elements_m {};

	Vect3() = default;
	Vect3(auto ... args): elements_m{Scalar(args) ... } {}
	virtual ~Vect3() = default;

	virtual void normalize() {/* Do nothing. */}

public:
	using ScalarType = Scalar;
	using DataType = decltype(elements_m);
	static unsigned constexpr Dimensions = 3;
	auto operator<=>(Vect3 const&) const = default;
	auto const& data() { return elements_m; };
};

class Coord3: public Vect3 {
public:
	Coord3(): Vect3{0, 0, 0} { normalize(); }
	Coord3(Scalar x, Scalar y, Scalar z): Vect3{x, y, z} { normalize(); }

	// Operators
	Coord3 operator+() const;
	Coord3 operator-() const;

	Coord3& operator+=(Coord3 const&);
	friend Coord3 operator+(Coord3 lhs, Coord3 const& rhs);

	Coord3& operator-=(Coord3 const&);
	friend Coord3 operator-(Coord3 lhs, Coord3 const& rhs);

	Coord3& operator*=(Scalar);
	friend Coord3 operator*(Coord3 lhs, Scalar rhs);
	friend Coord3 operator*(Scalar lhs, Coord3 rhs);

	Coord3& operator/=(Scalar);
	friend Coord3 operator/(Coord3 lhs, Scalar rhs);

	// Unified read/write syntax. One name is overloaded for both actions.
	[[nodiscard]] Scalar x() const;
	void x(Scalar);

	[[nodiscard]] Scalar y() const;
	void y(Scalar);

	[[nodiscard]] Scalar z() const;
	void z(Scalar);

	// On its own, "distance" is a measurement from the origin.
	[[nodiscard]] Scalar distance() const;
	void distance(Scalar);

	// You can also measure distance squared, for more efficient computation.
	[[nodiscard]] Scalar distance2() const;
	void distance2(Scalar);
};

class Norm3: public Coord3 {
public:
	Norm3(): Coord3{1, 0, 0} {}
	using Coord3::Coord3;

private:
	void normalize() override;
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

	// Properties
	[[nodiscard]] Coord3 const& origin() const;
	void origin(Coord3);

	[[nodiscard]] Norm3 const& direction() const;
	void direction(Norm3);
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

	// Properties
	[[nodiscard]] Coord3 const& origin() const;
	void origin(Coord3 c);

	[[nodiscard]] Norm3 const& direction() const;
	void direction(Norm3 n);

//	// Properties
//	Scalar length();

private:
	void normalize();
};

/* ~~ Matrices ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

class Matx4 {
	std::array<float, 16> elements_m;

public:
	Matx4(): elements_m {{
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 1.0, 0.0,
		0.0, 0.0, 0.0, 1.0,
	}} {}

	Matx4(std::array<float, 16> const& elements)
	:	elements_m{elements} {}

	auto operator[](unsigned i, unsigned j) const {
		return elements_m[4*i + j];
	}

	Matx4& operator*=(Matx4 const& other) { return *this = *this * other; }

	friend Matx4 operator*(Matx4 const&, Matx4 const&);

	static Matx4 RotateX(float);
	static Matx4 RotateY(float);
	static Matx4 RotateZ(float);

	auto const& data() { return elements_m; };
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
