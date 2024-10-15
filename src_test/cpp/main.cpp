#include <core/core_min.hpp>
#include <core/core_stream.hpp>
#include <core/string.hpp>
#include <iostream>
#include <object/object.hpp>

using namespace xel;
using namespace std;

namespace {}  // namespace

int main(int argc, char ** argv) {
	auto V = xVariable();
	V.UX   = 1024;
	V.UY   = 1024;
	V.U64  = XelBE64(V.U64);
	cout << hex << V.U64 << endl;
	cout << StrToHex(V.B, sizeof(V.B)) << endl;
	return 0;
}
