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
	using P = Permutation;
	P empty;
	EXPECT_EQ(empty, P());
	EXPECT_EQ(empty, P{});
	EXPECT_EQ(empty, P(P::Cycle ()));
	EXPECT_EQ(empty, P(P::Cycle {}));

	// Cycles of 0 or 1 elements do nothing, so are not canonical.
	using IL = std::initializer_list<P::Index>;
	EXPECT_EQ(empty, P(IL{}));
	EXPECT_EQ(empty, P(IL{}, IL{}));
	EXPECT_EQ(empty, P(IL{}, IL{}, IL{}));
	EXPECT_EQ(empty, P(IL{0}));
	EXPECT_EQ(empty, P(IL{0}, IL{}));
	EXPECT_EQ(empty, P(IL{}, IL{0}));
	EXPECT_EQ(empty, P(IL{0}, IL{}, IL{1}));
	EXPECT_EQ(empty, P(IL{1}, IL{2}, IL{3}));
	EXPECT_EQ(empty, P(IL{10000}));
}

TEST(BanditPermutation, CycleConstruction) {
	using P = Permutation;
	// Canonical representation on the left, per mathematical convention.
	EXPECT_EQ(P({5, 3}), P({3, 5}, {1}, {2}, {4}, {6}));
	EXPECT_EQ(P({6, 1, 2}, {5, 3}), P({1, 2, 6}, {3, 5}, {4}));
	EXPECT_EQ(P({6, 1, 2}, {5, 3}), P({1, 2, 6}, {3, 5}));
	EXPECT_EQ(P({6, 1, 2}, {5, 3}), P({2, 6, 1}, {5, 3}));
	EXPECT_EQ(P({6, 1, 2}, {5, 3}), P({2, 6, 1}, {5, 3}));
}

TEST(BanditPermutation, InvalidConstruction) {
	using P = Permutation;
	EXPECT_THROW(P({1, 1}), std::invalid_argument);
	EXPECT_THROW(P({1}, {1}), std::invalid_argument);
	EXPECT_THROW(P({2, 1}, {1}), std::invalid_argument);
	EXPECT_THROW(P({2, 1}, {1, 2}), std::invalid_argument);
	EXPECT_THROW(P({0, 1}, {0, 1}), std::invalid_argument);
	EXPECT_THROW(P({5, 6}, {4, 5}), std::invalid_argument);
}

TEST(BanditPermutation, Apply) {
	std::array<int, 8> const solved {0, 1, 2, 3, 4, 5, 6, 7};
	Permutation move ({2, 5, 1}, {7, 3});

	// [0, 1, 2, 3, 4, 5, 6, 7]
	//     ^  ^        ^        // These elements get cycled.
	// [0, 2, 5, 3, 4, 1, 6, 7]
	//           ^           ^  // As well as these.
	// [0, 2, 5, 7, 4, 1, 6, 3]
	std::array<int, 8> const scrambled {0, 2, 5, 7, 4, 1, 6, 3};

	EXPECT_EQ(scrambled, move.apply(solved));

	auto copy = solved;
	EXPECT_EQ(scrambled, move.apply_inplace(copy));

	EXPECT_THROW(move.apply(std::array<int, 7> {}), std::invalid_argument);
}

TEST(BanditPermutation, Inversion) {
	using P = Permutation;
	EXPECT_EQ(P().inverse(), P());
	EXPECT_EQ(P({0}).inverse(), P({0}));
	EXPECT_EQ(P({10, 20}).inverse(), P({20, 10}));
	EXPECT_EQ(P({1, 2, 3}).inverse(), P({3, 2, 1}));
	EXPECT_EQ(P({1, 2, 6}, {3, 5}).inverse(), P({6, 2, 1}, {5, 3}));

	std::array<int, 8> const solved {0, 1, 2, 3, 4, 5, 6, 7};
	P forward ({0, 2, 1, 3}, {4, 6, 5, 7});
	P backward = forward.inverse();

	EXPECT_EQ(solved, backward.apply(forward.apply(solved)));
	EXPECT_EQ(solved, forward.apply(backward.apply(solved)));
}

TEST(BanditPermutation, Composition) {
	using P = Permutation;
	P alpha ({10, 2, 4});
	P beta ({0, 11}, {8, 9});

	std::array<int, 12> const solved {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
	auto ab = solved;
	alpha.apply_inplace(ab); beta.apply_inplace(ab);
	auto ba = solved;
	beta.apply_inplace(ba); alpha.apply_inplace(ba);

	// We can apply with two function calls.
	EXPECT_EQ(ab, beta.apply(alpha.apply(solved)));
	EXPECT_EQ(ba, alpha.apply(beta.apply(solved)));

//	// We can also multiply twice.
//	EXPECT_EQ(ab, (solved * alpha) * beta);
//	EXPECT_EQ(ba, (solved * beta) * alpha);

//	// Or even move the parentheses around.
//	EXPECT_EQ(ab, solved * (alpha * beta));
//	EXPECT_EQ(ba, solved * (beta * alpha));
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

// Things to test:
// [ ]	- bandit executable
// [ ]	- puzzle language
// [ ]	- math library
// [ ]		- permutations
// [ ]		- coordinate library
// [ ]	- command line arguments parser
// [ ]	- utils
