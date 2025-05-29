#include <core/core_min.hpp>
#include <core/core_stream.hpp>
#include <core/string.hpp>
#include <iostream>
#include <object/object.hpp>

using namespace xel;
using namespace std;

namespace {}  // namespace

int main(int argc, char ** argv) {

	auto imm = xObjectIdManagerMini();
	X_RUNTIME_ASSERT(imm.Init());

	uint32_t Id_1    = 1;
	uint32_t Id_2    = 2;
	uint32_t Id_3    = 3;
	uint32_t Id_4095 = 4095;
	uint32_t Id_4096 = 4096;

	X_RUNTIME_ASSERT(imm.MarkInUse(Id_1));
	X_RUNTIME_ASSERT(!imm.MarkInUse(Id_1));
	X_RUNTIME_ASSERT(imm.MarkInUse(Id_2));
	imm.Release(Id_1);
	X_RUNTIME_ASSERT(imm.MarkInUse(Id_1));

	X_RUNTIME_ASSERT(imm.Acquire() == Id_3);

	X_RUNTIME_ASSERT(imm.MarkInUse(Id_4096));
	X_RUNTIME_ASSERT(!imm.MarkInUse(Id_4096));
	X_RUNTIME_ASSERT(imm.MarkInUse(Id_4095));
	imm.Release(Id_4096);
	X_RUNTIME_ASSERT(imm.MarkInUse(Id_4096));

	for (int i = 0; i < 4096 - 5; ++i) {
		auto Id = imm.Acquire();
		X_RUNTIME_ASSERT(Id);
		cout << Id << endl;
	}
	X_RUNTIME_ASSERT(!imm.Acquire());

	imm.Clean();

	cout << "done" << endl;
	return 0;
}
