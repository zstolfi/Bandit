#pragma once
#include <algorithm>
#include <print>
#include <ranges>
#include <string>
#include <cstdio>
#include <cstdlib>

template <class ... Args>
void error(std::format_string<Args ... > fmt, Args&& ... args) {
	std::print(stderr, fmt, std::forward<Args>(args) ... );
}

template <class ... Args>
void exit(std::format_string<Args ... > fmt, Args&& ... args) {
	std::print(stderr, fmt, std::forward<Args>(args) ... );
	std::exit(EXIT_FAILURE);
}
