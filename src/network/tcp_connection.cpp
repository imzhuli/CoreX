#include "./tcp_connection.hpp"

X_BEGIN

static ubyte KeepAliveRequestBuffer[PacketHeaderSize];
static ubyte KeepAliveBuffer[PacketHeaderSize];

static auto Init = xInstantRun([] {
	xPacketHeader::MakeRequestKeepAlive(KeepAliveRequestBuffer);
	xPacketHeader::MakeKeepAlive(KeepAliveBuffer);
});

void xTcpConnection::PostRequestKeepAlive() {
	PostData(KeepAliveRequestBuffer, PacketHeaderSize);
}

void xTcpConnection::PostKeepAlive() {
	PostData(KeepAliveBuffer, PacketHeaderSize);
}

X_END
