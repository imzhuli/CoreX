#include "./udp_channel.hpp"

X_BEGIN

static ubyte KeepAliveRequestBuffer[PacketHeaderSize];
static ubyte KeepAliveBuffer[PacketHeaderSize];

static auto Init = xInstantRun([] {
	xPacketHeader::MakeRequestKeepAlive(KeepAliveRequestBuffer);
	xPacketHeader::MakeKeepAlive(KeepAliveBuffer);
});

void xUdpChannel::PostRequestKeepAlive(const xNetAddress & DestiationAddress) {
	PostData(KeepAliveRequestBuffer, PacketHeaderSize, DestiationAddress);
}

void xUdpChannel::PostKeepAlive(const xNetAddress & DestiationAddress) {
	PostData(KeepAliveBuffer, PacketHeaderSize, DestiationAddress);
}

X_END
