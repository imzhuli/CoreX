#include <core/core_min.hpp>
#include <core/core_time.hpp>
#include <core/string.hpp>
#include <iostream>
#include <network/io_context.hpp>
#include <network/udp_channel.hpp>
#include <thread>

using namespace xel;
using namespace std;

static auto IC = xIoContext();
static auto UC = xUdpChannel();

struct xObserver : xUdpChannel::iListener {  // clang-format off
	void OnData(xUdpChannel * ChannelPtr, void * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress) {
		printf("OnData: from %s\n%s\n", RemoteAddress.ToString().c_str(), HexShow(DataPtr, DataSize).c_str());
	}
};
static auto OB = xObserver();

int main(int argc, char ** argv) {

	auto ICG           = xResourceGuard(IC);
	auto UCG           = xResourceGuard(UC, &IC, xNetAddress::Make4(), &OB);
	auto TargetAddress = xNetAddress::Parse("127.0.0.1:12345");

	// std::this_thread::sleep_for(xSeconds(10));
	auto Counter = size_t(0);
	auto T       = xTimer();
	while (true) {
		if (T.TestAndTag(xSeconds(5))) {
			char Buffer[256];
			auto RSize = snprintf(Buffer, SafeLength(Buffer), "Hello world! Counter=%zi", ++Counter);
			UC.PostData(Buffer, RSize, TargetAddress);
		}
		IC.LoopOnce();
	}

	return 0;
}
