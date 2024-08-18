#include <iostream>
#include <object/object.hpp>
using namespace xel;
using namespace std;

int main(int argc, char ** argv) {
	cout << "Max(mini): " << xObjectIdManagerMini::MaxObjectId << endl;
	cout << "Max: " << xObjectIdManager::MaxObjectId << endl;

	auto OIM = xObjectIdManagerMini();
	OIM.Init();

	for (uint64_t i = 0; i < 4400; ++i) {
		auto NewId = OIM.Acquire();
		// cout << "NewId: " << NewId << endl;
		Touch(NewId);
	}

	// for (uint64_t i = 1; i <= 4096; ++i) {
	// 	OIM.Release(i);
	// }

	OIM.Release(4096);
	OIM.Release(4000);
	OIM.Release(1);
	OIM.Release(3200);
	OIM.Release(3800);

	for (uint64_t i = 0; i < 5000; ++i) {
		auto NewId = OIM.Acquire();
		Touch(NewId);
		if (NewId) {
			cout << "--> NewId: " << NewId << endl;
		}
	}

	OIM.Clean();
	return 0;
}
