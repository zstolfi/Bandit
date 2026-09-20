#pragma once
#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <print>
#include <ranges>
#include <string>
#include <tuple>
#include <vector>
#include <cmath>
#include <cstdio>
#include <cstdlib>
namespace stdfs = std::filesystem;
namespace stdr = std::ranges;

template <class ... Args>
void error(std::format_string<Args ... > fmt, Args&& ... args) {
	std::print(stderr, fmt, std::forward<Args>(args) ... );
}

template <class ... Args>
void exit(std::format_string<Args ... > fmt, Args&& ... args) {
	std::print(stderr, fmt, std::forward<Args>(args) ... );
	std::exit(EXIT_FAILURE);
}
