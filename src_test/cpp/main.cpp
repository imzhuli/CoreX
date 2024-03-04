#include <algorithm/binary_exponentiation.hpp>
#include <atomic>
#include <core/core_min.hpp>
#include <core/core_value_util.hpp>
#include <core/version.hpp>
#include <iostream>

using namespace std;
using namespace xel;

int main(int argc, char ** argv) {

	auto TV = xVersion<int>();

	TV.EnableValue(123);
	cout << TV.GetVersion() << ": " << TV.Get() << endl;
	TV.Disable();
	cout << TV.GetVersion() << ": " << endl;
	TV.EnableValue();
	cout << TV.GetVersion() << ": " << TV.Get() << endl;

	return 0;
}