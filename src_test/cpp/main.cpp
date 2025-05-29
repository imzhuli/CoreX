#include <core/core_min.hpp>
#include <core/core_stream.hpp>
#include <core/indexed_storage.hpp>
#include <core/string.hpp>
#include <iostream>
#include <object/object.hpp>

using namespace xel;
using namespace std;

struct xSomething {
	char Buffer[77];
};

xIndexedStorage<xSomething, true> IS;

int main(int argc, char ** argv) {

	auto ISG = xResourceGuard(IS, 1024);
	X_RUNTIME_ASSERT(ISG);

	cout << "NodeSize=" << IS.NodeSize << endl;

	cout << hex << endl;

	auto Id0 = IS.Acquire();
	auto P0  = &IS[Id0];
	auto P1  = (xSomething *)((ubyte *)(P0) + IS.NodeSize);

	auto CheckedId0 = IS.GetObjectId(P0);

	cout << "Id0=" << Id0 << endl;
	cout << "CheckedId0=" << CheckedId0 << endl;

	cout << "P0=" << P0 << endl;
	cout << "P1=" << P1 << endl;
	auto CheckedId1 = IS.GetObjectId(P1);
	cout << "CheckedId1=" << CheckedId1 << endl;  // overflow
	X_RUNTIME_ASSERT(!CheckedId1);

	auto Id1 = IS.Acquire();
	auto Id2 = IS.Acquire();
	auto Id3 = IS.Acquire();
	IS.Release(Id3);
	IS.Release(Id2);
	IS.Release(Id1);
	CheckedId1 = IS.GetObjectId(P1);  // invalid key : recycled
	cout << "CheckedId1=" << CheckedId1 << endl;
	X_RUNTIME_ASSERT(!CheckedId1);

	Id1        = IS.Acquire();
	CheckedId1 = IS.GetObjectId(P1);  // invalid key : recycled
	cout << "Id1=" << Id1 << endl;
	cout << "CheckedId1=" << CheckedId1 << endl;
	return 0;
}
