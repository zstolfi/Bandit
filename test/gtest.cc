#include <gtest/gtest.h>
#include "math.hh"
#include "util.hh"

// Double parentheses required so our '<' isn't parsed as a less-than.
#define EXPECT_SAME_TYPE(T, U) EXPECT_TRUE((std::same_as<T, U>))
#define EXPECT_CONCEPT(C, ... ) EXPECT_TRUE((C<__VA_ARGS__>))

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

TEST(BanditPermutation, MemberTypes) {
	EXPECT_CONCEPT(std::integral, Permutation::Index);
	EXPECT_CONCEPT(stdr::range, Permutation::Cycle);
	EXPECT_CONCEPT(std::integral, stdr::range_value_t<Permutation::Cycle>);
}

TEST(BanditPermutation, EmptyConstruction) {
	Permutation p;
	EXPECT_EQ(p, Permutation ());
	EXPECT_EQ(p, Permutation {});
	EXPECT_EQ(p, Permutation ({}));
	EXPECT_EQ(p, Permutation {{}});
	EXPECT_EQ(p, Permutation (Permutation::Cycle {}));

	// Cycles of 0 or 1 elements do nothing.
	EXPECT_EQ(p, Permutation ({}));
	EXPECT_EQ(p, Permutation ({}, {}));
	EXPECT_EQ(p, Permutation ({}, {}, {}));
	EXPECT_EQ(p, Permutation ({0}));
	EXPECT_EQ(p, Permutation ({0}, {}));
	EXPECT_EQ(p, Permutation ({}, {0}));
	EXPECT_EQ(p, Permutation ({0}, {}, {1}));
	EXPECT_EQ(p, Permutation ({1}, {2}, {3}));
	EXPECT_EQ(p, Permutation ({10000}));
}

TEST(BanditPermutation, CycleConstruction) {
	using P = Permutation;
	// Canonical representation on the left, per mathematical convention.
	EXPECT_EQ(P ({5, 3}), P ({3, 5}, {1}, {2}, {4}, {6}));
	EXPECT_EQ(P ({6, 2, 1}, {5, 3}), P ({1, 2, 6}, {3, 5}, {4}));
	EXPECT_EQ(P ({6, 2, 1}, {5, 3}), P ({1, 2, 6}, {3, 5}));
	EXPECT_EQ(P ({6, 2, 1}, {5, 3}), P ({2, 6, 1}, {5, 5}));
}


TEST(BanditPermutation, InvalidConstruction) {
	EXPECT_THROW(Permutation ({1, 1}), std::invalid_argument);
	EXPECT_THROW(Permutation ({1}, {1}), std::invalid_argument);
	EXPECT_THROW(Permutation ({2, 1}, {1}), std::invalid_argument);
	EXPECT_THROW(Permutation ({2, 1}, {1, 2}), std::invalid_argument);
	EXPECT_THROW(Permutation ({0, 1}, {0, 1}), std::invalid_argument);
}

// composition

// inversion

// applying vs shuffling

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// Things to test:
// [ ]	- bandit executable
// [ ]	- puzzle language
// [ ]	- math library
// [ ]		- permutations
// [ ]		- coordinate library
// [ ]	- command line arguments parser
// [ ]	- utils
