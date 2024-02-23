#include <algorithm/binary_exponentiation.hpp>
#include <atomic>
#include <core/core_min.hpp>
#include <core/core_value_util.hpp>
#include <iostream>

using namespace std;
using namespace xel;

using xIntDummy = xDummy<sizeof(int)>;

int main(int argc, char ** argv) {

	int i = 123;

	auto & DummyPtrJ = xIntDummy::CastRef(i);
	DummyPtrJ.CreateValueAs<int>(456);
	cout << i << endl;

	return 0;
}