# ~~ Project Info ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #

# Modeled after: https://archive.ph/wOKUW
cmake_minimum_required(VERSION 3.17)

project(
	bandit
	VERSION 0.1.0
	LANGUAGES CXX C
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ~~ Directories ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #

set(INCLUDE_DIR ".include")
set(SOURCE_DIR "src")
set(TEST_DIR "test")

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

# GoogleTest
add_subdirectory(${INCLUDE_DIR}/googletest)

# ~~ Main Executable ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #

add_executable(${PROJECT_NAME})

target_sources(
	${PROJECT_NAME}
	PRIVATE
	${SOURCE_DIR}/main.cc
	${SOURCE_DIR}/vertex.glsl
	${SOURCE_DIR}/fragment.glsl
)

file(GLOB SHADERS "${SOURCE_DIR}/*.glsl")
file(COPY ${SHADERS} DESTINATION ${CMAKE_BINARY_DIR}/${PROJECT_NAME})

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
)

target_compile_options(
	${PROJECT_NAME}
	PRIVATE
	-Wall
	-fconcepts-diagnostics-depth=8
	-g
)

# ~~ Testing Executable ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ #

enable_testing()
add_executable(test-${PROJECT_NAME})

target_sources(
	test-${PROJECT_NAME}
	PRIVATE
	${TEST_DIR}/gtest.cc
)

target_link_libraries(
	test-${PROJECT_NAME}
	PRIVATE
	GTest::gtest_main
)

set_target_properties(
	test-${PROJECT_NAME}
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME}
)

include(GoogleTest)
gtest_discover_tests(test-${PROJECT_NAME})
