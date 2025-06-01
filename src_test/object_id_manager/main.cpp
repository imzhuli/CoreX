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

	OIM.Acquire(3800);

	for (uint64_t i = 0; i < 5000; ++i) {
		auto NewId = OIM.Acquire();
		Touch(NewId);
		if (NewId) {
			cout << "--> NewId: " << NewId << endl;
		}
	}

	OIM.Clean();

	auto imm = xObjectIdManager();
	X_RUNTIME_ASSERT(imm.Init());

	for (uint32_t Id = 1; Id <= imm.MaxObjectId; ++Id) {
		X_RUNTIME_ASSERT(!imm.IsInUse(Id));
	}

	uint32_t Id_1    = 1;
	uint32_t Id_2    = 2;
	uint32_t Id_3    = 3;
	uint32_t Id_4095 = 4095;
	uint32_t Id_4096 = 4096;

	X_RUNTIME_ASSERT(imm.Acquire(Id_1));
	X_RUNTIME_ASSERT(!imm.Acquire(Id_1));
	X_RUNTIME_ASSERT(imm.Acquire(Id_2));
	imm.Release(Id_1);
	X_RUNTIME_ASSERT(!imm.IsInUse(Id_1));
	X_RUNTIME_ASSERT(imm.Acquire(Id_1));
	X_RUNTIME_ASSERT(imm.IsInUse(Id_1));

	X_RUNTIME_ASSERT(imm.Acquire() == Id_3);
	X_RUNTIME_ASSERT(imm.IsInUse(Id_3));

	X_RUNTIME_ASSERT(!imm.IsInUse(Id_4096));
	X_RUNTIME_ASSERT(imm.Acquire(Id_4096));
	X_RUNTIME_ASSERT(imm.IsInUse(Id_4096));
	X_RUNTIME_ASSERT(!imm.Acquire(Id_4096));

	X_RUNTIME_ASSERT(imm.Acquire(Id_4095));
	imm.Release(Id_4096);
	X_RUNTIME_ASSERT(!imm.IsInUse(Id_4096));
	X_RUNTIME_ASSERT(imm.Acquire(Id_4096));

	for (unsigned i = 0; i < imm.MaxObjectId - 5; ++i) {
		auto Id = imm.Acquire();
		X_RUNTIME_ASSERT(Id);
		X_RUNTIME_ASSERT(imm.IsInUse(Id));
	}
	X_RUNTIME_ASSERT(!imm.Acquire());

	imm.Clean();

	cout << "done" << endl;
	return 0;
}
