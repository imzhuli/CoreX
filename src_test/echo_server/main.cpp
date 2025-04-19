#include <iostream>
#include <network/io_context.hpp>
#include <server_arch/service.hpp>

using namespace std;
using namespace xel;

struct xEchoService : xService {
	void OnClientConnected(xServiceClientConnection & Connection) override {
		cout << "OnClientConnected" << endl;
		//
	}
	bool OnPacket(xServiceClientConnection & Connection, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) override {
		auto Ret = xService::OnPacket(Connection, Header, PayloadPtr, PayloadSize);
		RuntimeAssert(Ret);

		do {
			PostData(Connection, PayloadPtr, PayloadSize);
		} while (false);

		return true;

		// ubyte Buffer[MaxPacketSize];
		// assert(xPacketHeader::Size + PayloadSize <= sizeof(Buffer));

		// auto SW = xStreamWriter(Buffer);
		// SW.Offset(PacketHeaderSize);
		// SW.W(PayloadPtr, PayloadSize);
		// auto RSize = SW.Offset();

		// auto RespHeader       = Header;
		// RespHeader.PacketSize = RSize;
		// RespHeader.Serialize(Buffer);

		// PostData(Connection, Buffer, RSize);
		// return Ret;
	}
	void OnClientClose(xServiceClientConnection & Connection) override {
		cout << "OnClientDisconnected" << endl;
		//
	}
};

static xIoContext   IoCtx;
static xEchoService EchoService;
static xNetAddress  ServerAddress = xNetAddress::Parse("0.0.0.0", 10000);

int main(int argc, char ** argv) {

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