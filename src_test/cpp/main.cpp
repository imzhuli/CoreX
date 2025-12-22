#include <core/core_min.hpp>
#include <core/core_stream.hpp>
#include <core/core_value_util.hpp>
#include <core/functional.hpp>
#include <core/indexed_storage.hpp>
#include <core/string.hpp>
#include <iostream>
#include <network/net_address.hpp>
#include <object/object.hpp>

using namespace xel;
using namespace std;

int main(int argc, char ** argv) {

	auto H = xHolder<std::vector<int>>();
	H.CreateValueWithList(3);

	auto & V = *H;
	RuntimeAssert(1 == V.size());

	return 0;
}
