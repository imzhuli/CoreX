#include <atomic>
#include <core/core_min.hpp>
#include <iostream>

using namespace std;
using namespace xel;

int foo() {
	return 100;
}

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

	return 0;
}