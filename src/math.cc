#include "math.hh"

namespace {
	void apply3(auto&& operation, auto& result, auto const& other) {
		for (unsigned i=0; i<3; i++) {
			result[i] = operation(result[i], other[i]);
		}
	}

	void apply3(auto&& operation, auto& result, Vect3::ValueType val) {
		for (unsigned i=0; i<3; i++) {
			result[i] = operation(result[i], val);
		}
	}
}

Coord3& Coord3::operator+=(Coord3 const& other) {
	apply3(std::plus {}, this->elements_m, other.elements_m);
	return *this;
}

Coord3 operator+(Coord3 lhs, Coord3 const& rhs) {
	return lhs += rhs;
}

Coord3& Coord3::operator-=(Coord3 const& other) {
	apply3(std::minus {}, this->elements_m, other.elements_m);
	return *this;
}

Coord3 operator-(Coord3 lhs, Coord3 const& rhs) {
	return lhs -= rhs;
}

Coord3& Coord3::operator*=(double val) {
	apply3(std::multiplies {}, this->elements_m, val);
	return *this;
}

Coord3 operator*(Coord3 lhs, double rhs) {
	return lhs *= rhs;
}

Coord3 operator*(double lhs, Coord3 rhs) {
	return rhs *= lhs;
}

Coord3& Coord3::operator/=(double val) {
	apply3(std::divides {}, this->elements_m, val);
	return *this;
}

Coord3 operator/(Coord3 lhs, double rhs) {
	return lhs /= rhs;
}

double Coord3::x() const { return elements_m[0]; }
void Coord3::x(double to) { elements_m[0] = to; }

double Coord3::y() const { return elements_m[1]; }
void Coord3::y(double to) { elements_m[1] = to; }

double Coord3::z() const { return elements_m[2]; }
void Coord3::z(double to) { elements_m[2] = to; }

// Properties
double Coord3::distance() const {
	return std::sqrt(distance2());
}

void Coord3::distance(double to) {
	auto from = distance();
	if (from != 0.0) {
		*this *= to/from;
	}
	else *this = {to, 0, 0};
}

double Coord3::distance2() const {
//	return (*this, *this).dot();
	return elements_m[0] * elements_m[0]
	+      elements_m[1] * elements_m[1]
	+      elements_m[2] * elements_m[2];
}

void Coord3::distance2(double to) {
	auto from = distance2();
	if (from != 0.0) {
		*this *= std::sqrt(to/from);
	}
	else *this = {std::sqrt(to), 0, 0};
}

void Norm3::normalize() {
	if (distance2() != 0.0) {
		double inv = std::pow(distance2(), -0.5);
		elements_m[0] *= inv;
		elements_m[1] *= inv;
		elements_m[2] *= inv;
	}
	else {
		bool strict = std::is_constant_evaluated();
		// Defining invalid normals at compile time is definitely an error.
		if (strict) throw std::domain_error {"Norm3 division by 0"};
		// Otherwise it's best to just keep our state valid.
		else *this = {};
	}
}

// Properties
Coord3 const& Ray3::origin() const {
	return origin_m;
}

void Ray3::origin(Coord3 o) {
	origin_m = o;
}

Norm3 const& Ray3::direction() const {
	return direction_m;
}

void Ray3::direction(Norm3 d) {
	direction_m = d;
}

Coord3 const& Plane3::origin() const { return origin_m; }
void Plane3::origin(Coord3 c) { origin_m = c; normalize(); }

Norm3 const& Plane3::direction() const { return direction_m; }
void Plane3::direction(Norm3 n) { direction_m = n; }

void Plane3::normalize() {
	double dist =
		origin_m.x() * direction_m.x() +
		origin_m.y() * direction_m.y() +
		origin_m.z() * direction_m.z()
	;
	origin_m = Coord3 {
		direction_m.x() * dist,
		direction_m.y() * dist,
		direction_m.z() * dist,
	};
}

/* ~~ Relational Properties ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

double RegularParameters<Coord3, Coord3>::distance2() const {
	Coord3 diff = get<0>() - get<1>();
	return diff.x() * diff.x()
	+      diff.y() * diff.y()
	+      diff.z() * diff.z();
}

RegularParameters<Coord3, Coord3> operator,(Coord3 a, Coord3 b) {
	return RegularParameters {a, b};
}


/* ~~ Formatting & Printing ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

std::ostream& operator<<(std::ostream& os, Permutation const& p) {
	return os << std::format("{}", p);
}
