#include "math.hh"
#include "util.hh"

int main() {
	Coord3 position {1, 2, 3};
	std::print("[{}, {}, {}]\n", position.x(), position.y(), position.z());

	auto squared = [] (auto n) { return n*n; };

	double value = position.x();
	position.x(0);
	position.y(squared);
	position.z(Becomes *= -100);
	std::print("[{}, {}, {}]\n", position.x(), position.y(), position.z());

	std::print("{}\n", position.length());
//	position.length(position.length() * 2) // Calls sqrt() multiple times.
	position.length(Becomes *= 2); // Calls sqrt() never :)
	std::print("{}\n", position.length());

	Norm3 normal {position};
	std::print("[{}, {}, {}]\n", normal.x(), normal.y(), normal.z());
	position = Norm3 {position};
	std::print("[{}, {}, {}]\n", position.x(), position.y(), position.z());
//	position.direction(Becomes *= -1);
//	position.direction([] (auto n) { return -n; });
//	std::print("[{}, {}, {}]\n", position.x(), position.y(), position.z());
}
