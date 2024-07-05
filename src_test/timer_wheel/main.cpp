#include <core/timer_wheel.hpp>
#include <iostream>
#include <thread>

using namespace std;
using namespace xel;

struct xAA : xTimerWheelNode {
	uint64_t T = 0;
};

void Test(xVariable Context, xTimerWheelNode * NP, uint64_t TimestampMS) {
	auto P = (xAA *)NP;
	cout << "C: " << Context.I << " T:" << P->T << " : " << TimestampMS << endl;
}

xAA AA;
xAA AA1;
xAA A1;
xAA A5;

int main(int argc, char ** argv) {

	cout << "Hello world" << endl;

	xTimerWheel TW;
	TW.Init(5);

	TW.ScheduleByOffset(AA, { Test, { .I = 0 } }, 0);
	TW.ScheduleByOffset(AA1, { Test, { .I = 1 } });
	TW.ScheduleByTimeoutMS(A1, { Test, { .I = 1 } }, 1);
	TW.ScheduleByTimeoutMS(A5, { Test, { .I = 5 } }, 5);
	for (size_t i = 0; i < 10; ++i) {
		cout << "Loop: " << i << endl;
		TW.Forward();
		std::this_thread::sleep_for(chrono::milliseconds(1));
	}

	TW.Clean();
	return 0;
}
