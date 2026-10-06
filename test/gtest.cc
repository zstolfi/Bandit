#include <gtest/gtest.h>
#include "math.hh"
#include "util.hh"

// Double parentheses required so our '<' isn't parsed as a less-than.
#define EXPECT_SAME_TYPE(T, U) EXPECT_TRUE((std::same_as<T, U>))
#define EXPECT_CONCEPT(C, ... ) EXPECT_TRUE((C<__VA_ARGS__>))

/* ~~ 3D Coordinates ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

TEST(Bandit3DCoordinate, CompiletimeProperties) {
	EXPECT_CONCEPT(std::regular, Coord3);
	EXPECT_EQ(Coord3::Dimension(), 3);
	EXPECT_CONCEPT(std::convertible_to, Coord3::ScalarType, double);
}

TEST(Bandit3DCoordinate, EmptyConstruction) {
	Coord3 empty;
	EXPECT_EQ(empty, Coord3 ());
	EXPECT_EQ(empty, Coord3 {});
	EXPECT_EQ(empty, Coord3 (0, 0, 0));
	EXPECT_EQ(empty,(Coord3 {0, 0, 0}));
	EXPECT_EQ(empty, Coord3 ({0, 0, 0}));
}

TEST(Bandit3DCoordinate, Construction) {
	Coord3 c {1, 2, 3};
	using D = Coord3::DataType;
	EXPECT_EQ(c.data(), (D {1, 2, 3}));
}

TEST(Bandit3DCoordinate, Position) {
	Coord3 c {-1, -2, -3};
	// These are [[nodiscard]], but I don't know how to test for that.
	EXPECT_EQ(c.x(), -1);
	EXPECT_EQ(c.y(), -2);
	EXPECT_EQ(c.z(), -3);

	using D = Coord3::DataType;
	c.x(1);
	EXPECT_EQ(c.data(), (D { 1, -2, -3}));
	c.y(2);
	EXPECT_EQ(c.data(), (D { 1,  2, -3}));
	c.z(3);
	EXPECT_EQ(c.data(), (D { 1,  2,  3}));
}

TEST(Bandit3DCoordinate, UnaryPlus) {
	Coord3 c {1, 2, 3};

	using D = Coord3::DataType;
	EXPECT_EQ((+c).data(), (D {1, 2, 3}));
	EXPECT_EQ(c.data(), (D {1, 2, 3}));
	c = +c;
	EXPECT_EQ(c.data(), (D {1, 2, 3}));
}

TEST(Bandit3DCoordinate, UnaryMinus) {
	Coord3 c {1, 2, 3};

	using D = Coord3::DataType;
	EXPECT_EQ((-c).data(), (D {-1, -2, -3}));
	EXPECT_EQ(c.data(), (D {1, 2, 3}));
	c = -c;
	EXPECT_EQ(c.data(), (D {-1, -2, -3}));
}

TEST(Bandit3DCoordinate, CoordinateAddition) {
	Coord3 a {1, 2, 3}, b {8, 7, 6};

	using D = Coord3::DataType;
	EXPECT_EQ((a + b).data(), (D {9, 9, 9}));
	a += a;
	EXPECT_EQ(a.data(), (D {2, 4, 6}));
}

TEST(Bandit3DCoordinate, CoordinateSubtraction) {
	Coord3 a {1, 2, 3}, b {8, 7, 6};

	using D = Coord3::DataType;
	EXPECT_EQ((a - b).data(), (D {-7, -5, -3}));
	a -= a;
	EXPECT_EQ(a.data(), (D {0, 0, 0}));
}

TEST(Bandit3DCoordinate, ScalarMultiplication) {
	Coord3 c {2, 5, 1};

	using D = Coord3::DataType;
	EXPECT_EQ((2 * c).data(), (D {4, 10, 2}));
	EXPECT_EQ((c * 3).data(), (D {6, 15, 3}));
	c *= 1.5;
	EXPECT_EQ(c.data(), (D {3, 7.5, 1.5}));
}

TEST(Bandit3DCoordinate, ScalarDivision) {
	Coord3 c {3, 6, 9};

	using D = Coord3::DataType;
	EXPECT_EQ((c / 2).data(), (D {1.5, 3, 4.5}));
	c /= 3;
	EXPECT_EQ(c.data(), (D {1, 2, 3}));
}

TEST(Bandit3DCoordinate, Length) {
	Coord3 position {};
	Coord3::ScalarType const Epsilon {1e-9}; // one billionth

	// Reading
	position = {0, 0, 0};
	EXPECT_NEAR(position.length() , 0, Epsilon);
	EXPECT_NEAR(position.length2(), 0, Epsilon);

	position = {1, 0, 0};
	EXPECT_NEAR(position.length() , 1, Epsilon);
	EXPECT_NEAR(position.length2(), 1, Epsilon);

	position = {3, 4, 12}; // https://math.stackexchange.com/a/2863663
	EXPECT_NEAR(position.length() , 13, Epsilon);
	EXPECT_NEAR(position.length2(), 169, Epsilon);

	position = {0, 1, -1};
	EXPECT_NEAR(position.length() , std::sqrt(2), Epsilon);
	EXPECT_NEAR(position.length2(), 2           , Epsilon);

	position = {2, 5, 1};
	EXPECT_NEAR(position.length() , std::sqrt(30), Epsilon);
	EXPECT_NEAR(position.length2(), 30           , Epsilon);

	// Writing
	position.length(9);
	EXPECT_NEAR(position.length() , 9 , Epsilon);
	EXPECT_NEAR(position.length2(), 81, Epsilon);

	position.length(0);
	EXPECT_NEAR(position.length() , 0, Epsilon);
	EXPECT_NEAR(position.length2(), 0, Epsilon);

	position.length2(25);
	EXPECT_NEAR(position.length() , 5 , Epsilon);
	EXPECT_NEAR(position.length2(), 25, Epsilon);

	position.length2(0);
	EXPECT_NEAR(position.length() , 0, Epsilon);
	EXPECT_NEAR(position.length2(), 0, Epsilon);
}

TEST(Bandit3DCoordinatePair, Distance) {
	Coord3 me {0, 0, 0};
	Coord3 you {0, 0, 0};
	EXPECT_EQ((me, you).distance(), 0);
	EXPECT_EQ((me, you).distance2(), 0);

	Coord3::ScalarType n = 1'000'000;
	me = {n, n, n};
	you = {n, n, n};
	EXPECT_EQ((me, you).distance(), 0);
	EXPECT_EQ((me, you).distance2(), 0);

	me = {2, 2, 2};
	you = {-1, -1, -1};
	EXPECT_EQ((me, you).distance(), std::sqrt(27));
	EXPECT_EQ((me, you).distance2(), 27);

	me = {3, 6, 9};
	you = {-2, -5, -1};
	EXPECT_EQ((me, you).distance(), std::sqrt(246));
	EXPECT_EQ((me, you).distance2(), 246);
}

TEST(Bandit3DCoordinatePair, DotProduct) {
	Coord3 a {0, 0, 0};
	Coord3 b {0, 0, 0};
	EXPECT_EQ((a, b).dot(), 0);

	a = {0, 0, 1};
	b = {1, 0, 0};
	EXPECT_EQ((a, b).dot(), 0);

	a = {1, 0, 0};
	b = {0, -1, -1};
	EXPECT_EQ((a, b).dot(), 0);

	a = {1, 2, 3};
	b = {-3, -2, -1};
	EXPECT_EQ((a, b).dot(), -10);

	a = {8, 6, 7};
	b = {3, 0, 9};
	EXPECT_EQ((a, b).dot(), 87);
}

/* ~~ 3D Normalized Coordinates ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

TEST(Bandi3DtNormal, CompiletimeProperties) {
	EXPECT_CONCEPT(std::regular, Coord3);
	EXPECT_EQ(Norm3::Dimension(), 3);
	EXPECT_CONCEPT(std::convertible_to, Norm3::ScalarType, double);
}


/* ~~ Permutations ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

TEST(BanditPermutation, CompiletimeProperties) {
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
