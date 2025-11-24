#include <core/core_min.hpp>
#include <core/core_stream.hpp>
#include <core/functional.hpp>
#include <core/indexed_storage.hpp>
#include <core/string.hpp>
#include <iostream>
#include <network/net_address.hpp>
#include <object/object.hpp>

using namespace xel;
using namespace std;

int main(int argc, char ** argv) {

	auto P = new xIndexedStorageStatic<int, 128>();
	for (int I = 0; I < 130; ++I) {
		auto Key = P->Acquire(130 - I);
		if (!Key) {
			break;
		}
		auto & V = (*P)[Key];
		cout << "I=" << I << ", Index=" << Key.GetIndex() << ", Value=" << V << endl;
	}
	delete Steal(P);

	return 0;
}
