#include <core/core_value_util.hpp>
#include <iostream>
#include <vector>

using namespace xel;
using namespace std;

int main(int argc, char ** argv) {

	auto H = xAutoHolder<std::vector<int>>();
	cout << H->size() << endl;

	auto H1 = xAutoHolder<std::vector<int>>(3);
	cout << H1->size() << endl;

	auto H2 = xAutoHolder<std::vector<int>>{ 1, 2, 3, 4, 5 };
	cout << H2->size() << endl;

	H2.Reset();
	cout << H2->size() << endl;

	H2.ResetValue(123);
	cout << H2->size() << endl;

	H2.ResetValueWithList(1, 2);
	cout << H2->size() << endl;

	return 0;
}
