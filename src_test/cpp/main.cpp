#include <core/core_min.hpp>
#include <iostream>
#include <object/object.hpp>

using namespace xel;
using namespace std;

class Deleter {
public:
	void operator()(xObjectBase * OP) { cout << "Deleter: " << (void *)OP << endl; }
};

using xH = xObjectHolder<Deleter>;

int main(int argc, char ** argv) {

	xObjectBase B1 = {};
	xObjectBase B2 = {};

	xH H = { &B1 };

	cout << (B1 == B2) << endl;
	cout << (B1 != B2) << endl;

	return 0;
}
