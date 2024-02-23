#include <algorithm/binary_exponentiation.hpp>
#include <atomic>
#include <core/core_min.hpp>
#include <iostream>

using namespace std;
using namespace xel;

static char C = 0;

struct S {

	int i = 1024;
	int j = 128;

	operator char &() {
		return C;
	}
	operator const char &() const {
		return C;
	}
};

int main(int argc, char ** argv) {
	S s = { 3 };
	ConstructValueWithList(s, 2, 3);
	cout << s.i << " " << s.j << endl;

	cout << (void *)&s << endl;
	cout << (void *)&(char &)s << endl;
	cout << (void *)&static_cast<char &>(s) << endl;
	cout << (void *)&reinterpret_cast<char &>(s) << endl;
	cout << (void *)AddressOf(s) << endl;

	return 0;
}