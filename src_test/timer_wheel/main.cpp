#include <core/timer_wheel.hpp>
#include <iostream>
#include <thread>

using namespace std;
using namespace xel;

struct xAA : xTimerWheelNode {
	uint64_t T = 0;
};

void Test(xTimerWheelNode * NP, uint64_t TimestampMS) {
	auto P = (xAA *)NP;
	cout << "T:" << P->T << endl;
}

xAA AA;

int main(int argc, char ** argv) {

	cout << "Hello world" << endl;

	xTimerWheel TW;
	TW.Init(2);

	TW.ScheduleByOffset(&AA, 1, Test);
	for (size_t i = 0; i < 10; ++i) {
		TW.Forward();

		std::this_thread::sleep_for(chrono::milliseconds(1));
	}

	TW.Clean();
	return 0;
}
