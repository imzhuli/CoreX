#include <iostream>
#include <network/io_context.hpp>
#include <server_arch/service.hpp>

using namespace std;
using namespace xel;

struct xEchoService : xService {
	bool OnPacket(xServiceClientConnection & Connection, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) override {
		auto Ret = xService::OnPacket(Connection, Header, PayloadPtr, PayloadSize);

		ubyte Buffer[128];
		auto  Size = xPacketHeader::MakeKeepAlive(Buffer);
		PostData(Connection, Buffer, Size);
		return Ret;
	}
};

static xIoContext   IoCtx;
static xEchoService EchoService;
static xNetAddress  ServerAddress = xNetAddress::Parse("0.0.0.0", 10000);

int main(int argc, char * argv[]) {

	auto IR = xResourceGuard(IoCtx);
	if (!IR) {
		cerr << "Invalid IR" << endl;
		return -1;
	}
	auto SR = xResourceGuard(EchoService, &IoCtx, ServerAddress, true);
	if (!SR) {
		cerr << "Invalid SR" << endl;
		return -1;
	}

	while (true) {
		IoCtx.LoopOnce();
		EchoService.Tick();
	}

	return 0;
}