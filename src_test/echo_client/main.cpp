#include <core/core_time.hpp>
#include <iostream>
#include <network/io_context.hpp>
#include <server_arch/client.hpp>

using namespace std;
using namespace xel;

struct xHelloWorldClient : xClient {};

static xIoContext        IoCtx;
static xHelloWorldClient Client;
static xNetAddress       ServerAddress = xNetAddress::Parse("127.0.0.1", 10000);

int main(int argc, char ** argv) {
	auto IR = xResourceGuard(IoCtx);
	if (!IR) {
		cerr << "Invalid IR" << endl;
		return -1;
	}
	auto CR = xResourceGuard(Client, &IoCtx, ServerAddress, xNetAddress::Make4());
	if (!CR) {
		cerr << "Invalid CR" << endl;
		return -1;
	}
	Client.SetKeepAliveTimeout(60'000);

	xTimer Timer;
	while (true) {
		auto NowMS = GetTimestampMS();
		IoCtx.LoopOnce();
		Client.Tick(NowMS);
	}
	return 0;
}
