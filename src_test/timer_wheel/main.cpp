#include <core/timer_wheel.hpp>
#include <iostream>
#include <thread>

using namespace std;
using namespace xel;

struct xAA : xTimerWheelNode {
	uint64_t T = 0;
};

void Test(xVariable Context, uint64_t TimestampMS) {
	cout << "C: " << Context.I << " T:" << " : " << TimestampMS << endl;
}

xAA AA;
xAA AA1;
xAA A1;
xAA A5;

xTimerWheel     TW;
xTimerWheelNode N;

void IL(xVariable Context, uint64_t TimestampMS) {
	cout << "IL: " << Context.I << endl;
	if (Context.I++ < 10) {
		TW.RescheduleByOffset(N, { IL, Context }, 0);
	}
}

void SN(xVariable Context, uint64_t TimestampMS) {
	cout << "SN: " << Context.I << endl;
	if (Context.I++ < 10) {
		TW.RescheduleNext(N, { SN, Context });
	}
}

int main(int argc, char ** argv) {

	cout << "Hello world" << endl;

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

	// IL:
	SetCallback(N, { IL, { .I = 0 } });
	TW.ScheduleByTimeoutMS(N, 0);
	TW.Forward();
	TW.ScheduleNext(N, { SN, {} });
	TW.Forward();
	TW.Forward();
	TW.Forward();

	TW.Clean();
	return 0;
}
