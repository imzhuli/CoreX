#include <algorithm/binary_exponentiation.hpp>
#include <atomic>
#include <core/core_min.hpp>
#include <iostream>

using namespace std;
using namespace xel;

int foo() {
	return 100;
}

struct Fibonacci {
	using xPair = struct {
		uintmax_t Fn, Fn1;
	};
	using xUnit = struct {
		uintmax_t m00, m01;
		uintmax_t m10, m11;
	};
	static const xUnit Transform;
};

static Fibonacci::xUnit operator*(const Fibonacci::xUnit & L, const Fibonacci::xUnit & R) {
	return {
		L.m00 * R.m00 + L.m01 * R.m10,
		L.m00 * R.m01 + L.m01 * R.m11,
		L.m10 * R.m00 + L.m11 * R.m10,
		L.m10 * R.m01 + L.m11 * R.m11,
	};
}

Fibonacci::xPair operator*(const Fibonacci::xPair & Pair, const Fibonacci::xUnit & Transform) {
	return { Pair.Fn * Transform.m00 + Pair.Fn1 * Transform.m10, Pair.Fn * Transform.m01 + Pair.Fn1 * Transform.m11 };
}

// clang-format off
const Fibonacci::xUnit Unit = {
    1, 0,
    0, 1,
};
const Fibonacci::xUnit Transform = {
	0, 1, 
	1, 1,
};
// clang-format on

int main(int argc, char ** argv) {
	int i = 100;

	cout << MakeSigned(100U) << endl;
	cout << MakeUnsigned(-100) << endl;
	do {
		auto IGuard  = xValueGuard(i);
		i            = 1000;
		auto IGuard2 = std::move(IGuard);
	} while (false);
	cout << "I = " << i << endl;

	// BinaryExponentiation
	do {
		auto TwoE10 = BinaryExponentiation(2, 10);
		cout << TwoE10 << endl;

		auto TwoE10U = BinaryExponentiation(2, 10, 1);
		cout << TwoE10U << endl;

		auto F0 = Fibonacci::xPair{ 1, 1 } * BinaryExponentiation(Transform, 0, Unit);
		cout << "F_0"
			 << ": " << F0.Fn << endl;
		auto F1 = Fibonacci::xPair{ 1, 1 } * BinaryExponentiation(Transform, 1);
		cout << "F_1"
			 << ": " << F1.Fn << endl;
		auto F_2 = Fibonacci::xPair{ 1, 1 } * BinaryExponentiation(Transform, 2);
		cout << "F_2"
			 << ": " << F_2.Fn << endl;
		auto F_3 = Fibonacci::xPair{ 1, 1 } * BinaryExponentiation(Transform, 3);
		cout << "F_3"
			 << ": " << F_3.Fn << endl;

		auto F32 = Fibonacci::xPair{ 1, 1 } * BinaryExponentiation(Transform, 32);
		cout << "F_32"
			 << ": " << F32.Fn << endl;

	} while (false);

	return 0;
}