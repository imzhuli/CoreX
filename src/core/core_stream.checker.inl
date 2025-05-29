#pragma once
#include "./core_stream.hpp"

#ifdef __X_CORE_CHECKER__

X_BEGIN
namespace __detail__::__checkers__ {

	union xConsistentOrderingTest {
		ubyte   BS[4];
		int32_t I;
		float   F;
	};
	static_assert(sizeof(xConsistentOrderingTest) == 4);

	static auto RealConsistentOrderingChecker = xInstantRun([] {
		auto TI = xConsistentOrderingTest();
		auto TF = xConsistentOrderingTest();
		TI.I    = 0x01;
		TF.F    = 0x01;

		bool LE = (TI.BS[0] == 0x01 && TF.BS[0] == 0x00);
		bool BE = (TI.BS[0] == 0x00 && TF.BS[0] == 0x3F);

		X_RUNTIME_ASSERT(LE ^ BE);
	});

}  // namespace __detail__::__checkers__
X_END

#endif
