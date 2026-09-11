# ~~ Project Info ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #

# Modeled after: https://archive.ph/wOKUW
cmake_minimum_required(VERSION 3.17)

project(
	bandit
	VERSION 0.1.0
	LANGUAGES CXX C
)

# ~~ Directories ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #

set(INCLUDE_DIR ".include")
set(SOURCE_DIR "src")

# ~~ Dependencies ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #

# OpenGL
set(OpenGL_GL_PREFERENCE GLVND)

# GLFW
option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
option(GLFW_INSTALL OFF)
find_package(glfw3 REQUIRED)

# GLAD
add_library(
	glad
	STATIC
	${INCLUDE_DIR}/glad/src/glad.c
)

target_include_directories(
	glad
	PUBLIC
	${INCLUDE_DIR}/glad/include
)

# ~~ Project ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #

add_executable(${PROJECT_NAME})

target_sources(
	${PROJECT_NAME}
	PRIVATE
	${SOURCE_DIR}/main.cc
)

target_link_libraries(
	${PROJECT_NAME}
	PRIVATE
	glfw
	glad
	${CMAKE_DL_LIBS} # Required for GLAD.
)

set_target_properties(
	${PROJECT_NAME}
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
	COMPILE_OPTIONS -Werror
	CXX_STANDARD 23
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)
