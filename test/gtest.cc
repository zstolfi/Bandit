#include <gtest/gtest.h>
#include "math.hh"
#include "util.hh"

// Double parentheses required so our '<' isn't parsed as a less-than.
#define EXPECT_SAME_TYPE(T, U) EXPECT_TRUE((std::same_as<T, U>))
#define EXPECT_CONCEPT(C, ... ) EXPECT_TRUE((C<__VA_ARGS__>))

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

TEST(BanditPermutation, MemberTypes) {
	using P = Permutation;
	EXPECT_CONCEPT(std::integral, P::Index);
	EXPECT_CONCEPT(stdr::range, P::Cycle);
	EXPECT_CONCEPT(std::same_as, P::Index, stdr::range_value_t<P::Cycle>);
}

TEST(BanditPermutation, EmptyConstruction) {
	Permutation p;
	EXPECT_EQ(p, Permutation ());
	EXPECT_EQ(p, Permutation {});
	EXPECT_EQ(p, Permutation (Permutation::Cycle ()));
	EXPECT_EQ(p, Permutation (Permutation::Cycle {}));

	// Cycles of 0 or 1 elements do nothing.
	using IL = std::initializer_list<Permutation::Index>;
	EXPECT_EQ(p, Permutation (IL{}));
	EXPECT_EQ(p, Permutation (IL{}, IL{}));
	EXPECT_EQ(p, Permutation (IL{}, IL{}, IL{}));
	EXPECT_EQ(p, Permutation (IL{0}));
	EXPECT_EQ(p, Permutation (IL{0}, IL{}));
	EXPECT_EQ(p, Permutation (IL{}, IL{0}));
	EXPECT_EQ(p, Permutation (IL{0}, IL{}, IL{1}));
	EXPECT_EQ(p, Permutation (IL{1}, IL{2}, IL{3}));
	EXPECT_EQ(p, Permutation (IL{10000}));
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
