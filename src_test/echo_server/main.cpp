#include <iostream>
#include <server_arch/tcp_service.hpp>
#include <server_arch/udp_service.hpp>

using namespace std;
using namespace xel;

static xIoContext  IoCtx;
static xTicker     Ticker;
static xNetAddress ServerAddress = xNetAddress::Parse("0.0.0.0:10000");
static xTcpService TcpService;

int main(int argc, char ** argv) {

	auto IR = xResourceGuard(IoCtx);
	if (!IR) {
		cerr << "Invalid IR" << endl;
		return -1;
	}

	TcpService.OnClientPacket = [](const xTcpServiceClientConnectionHandle & Handle, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr,
								   size_t PayloadSize) {
		ubyte Buffer[MaxPacketSize];

		auto SW = xStreamWriter(Buffer);
		SW.Offset(PacketHeaderSize);
		SW.W(PayloadPtr, PayloadSize);
		auto RSize = SW.Offset();

		auto RespHeader = xPacketHeader{
			.CommandId = CommandId,
			.RequestId = RequestId,
		};
		RespHeader.PacketSize = RSize;
		RespHeader.Serialize(Buffer);

		Handle.PostData(Buffer, RSize);
		return true;
	};

	while (true) {
		Ticker.Update();
		IoCtx.LoopOnce();
		TcpService.Tick(Ticker());
	}

	return 0;
}