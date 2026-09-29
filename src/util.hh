#pragma once
#include <algorithm>
#include <array>
#include <concepts>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <print>
#include <ranges>
#include <set>
#include <span>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>
#include <cmath>
#include <cstdio>
#include <cstdlib>
namespace stdfs = std::filesystem;
namespace stdr = std::ranges;
namespace stdv = std::views;

/* ~~ Logging ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

template <class ... Args>
void error(std::format_string<Args ... > fmt, Args&& ... args) {
	std::print(stderr, fmt, std::forward<Args>(args) ... );
}

template <class ... Args>
void exit(std::format_string<Args ... > fmt, Args&& ... args) {
	std::print(stderr, fmt, std::forward<Args>(args) ... );
	std::exit(EXIT_FAILURE);
}

/* ~~ Concepts ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

template <class C, class T>
concept IsRangeOf =
	stdr::range<std::remove_cvref_t<C>> &&
	std::convertible_to<
		stdr::range_value_t<std::remove_cvref_t<C>>,
		T
	>
;

/* ~~ Math Helpers ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// Regular parameters are used in Bandit's linear algebra library.
// An example would look something like this:

//	Point a, b;
//	std::cout << a.location();
//	std::cout << b.location();
//	std::cout << (a, b).distance();

// (a, b) defines a value of type RegularParameters<Point, Point>.
template <class ... >
struct RegularParameters {};

template <class ... Args>
RegularParameters(Args ... ) -> RegularParameters<Args ... >;

// To implement properties of regular parameters, we specialize like so:

//	template <>
//	struct RegularParameters<Line, Plane>
//	:	RpOverload<Line, Plane> {
//		using RpOverload<Line, Plane>::RpOverload;
//
//		auto intersection() {/* ... */}
//	};
//
//	auto operator,(Line a, Plane b) { return RegularParameters {a, b}; }

// Now the following call is possible:

//	(line, plane).intersection();

template <class ... Args>
struct RpOverload: public std::tuple<Args ... > {
	using std::tuple<Args ... >::tuple;

protected:
	template <auto I>
	auto const& get() const { return std::get<I>(*this); }

	template <class T>
	auto const& get() const { return std::get<T>(*this); }
};
