#include <iostream>
#include <object/object.hpp>
using namespace xel;
using namespace std;

int main(int argc, char ** argv) {
	cout << "Max: " << xObjectIdManager::MaxObjectId << endl;

	auto OIM = xObjectIdManager();
	OIM.Init();

	for (uint64_t i = 0; i < 262148; ++i) {
		auto NewId = OIM.Acquire();
		Touch(NewId);
	}

	for (uint64_t i = 1; i <= 262144; ++i) {
		OIM.Release(i);
	}

	// OIM.Release(262144);
	// OIM.Release(262100);
	// OIM.Release(1);

	for (uint64_t i = 0; i < 262148; ++i) {
		auto NewId = OIM.Acquire();
		Touch(NewId);
		if (NewId) {
			// cout << "--> NewId: " << NewId << endl;
		}
	}

	OIM.Clean();
	return 0;
}
