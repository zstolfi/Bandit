cmake_minimum_required(VERSION 3.14)
set(CMAKE_CXX_STANDARD 23)

project(
	Bandit
	VERSION 0.1.0
	LANGUAGES CXX
)

add_executable(
	bandit
	main.cc
)
