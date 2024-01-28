#include <atomic>
#include <core/core_min.hpp>
#include <iostream>

using namespace std;
using namespace xel;

int main(int argc, char ** argv) {

	int i = 100;
	do {
		auto IGuard  = xValueGuard(i);
		i            = 1000;
		auto IGuard2 = std::move(IGuard);
	} while (false);
	cout << "I = " << i << endl;

	return 0;
}