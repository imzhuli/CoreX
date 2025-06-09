#include <core/core_min.hpp>
#include <core/core_stream.hpp>
#include <core/indexed_storage.hpp>
#include <core/string.hpp>
#include <iostream>
#include <object/object.hpp>

using namespace xel;
using namespace std;

int main(int argc, char ** argv) {

	int A[4] = { 1, 2, 3, 4 };
	ZeroFill(A);

	for (auto & I : A) {
		cout << I << endl;
	}

	cout << hex << xObjectBase::ID_INDEX_MASK << endl;
	cout << hex << xObjectBase::ID_FLAG_MASK << endl;

	cout << hex << xObjectBase::ID_INDEX_MASK + xObjectBase::ID_FLAG_MASK << endl;

	return 0;
}
