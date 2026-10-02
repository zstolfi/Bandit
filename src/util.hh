#pragma once

// STL
#include <algorithm>
#include <array>
#include <concepts>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <optional>
#include <print>
#include <ranges>
#include <set>
#include <span>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <vector>

// C library
#include <cmath>
#include <cstdio>
#include <cstdlib>

// Shorthands
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
