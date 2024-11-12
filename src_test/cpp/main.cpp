#include <core/core_min.hpp>
#include <core/core_stream.hpp>
#include <core/string.hpp>
#include <iostream>
#include <object/object.hpp>

using namespace xel;
using namespace std;

namespace {}  // namespace

int main(int argc, char ** argv) {

	auto s    = string("h,w,");
	auto segs = Split(s, ",");
	cout << segs.size() << endl;

	for (auto & r : segs) {
		cout << r << endl;
	}

	return 0;
}
