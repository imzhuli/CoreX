#include "./tcp_connection.hpp"

X_BEGIN

static ubyte KeepAliveRequestBuffer[PacketHeaderSize];
static ubyte KeepAliveBuffer[PacketHeaderSize];

static auto Init = xInstantRun([] {
	xPacketHeader::MakeRequestKeepAlive(KeepAliveRequestBuffer);
	xPacketHeader::MakeKeepAlive(KeepAliveBuffer);
});

xNetAddress xTcpConnection::GetRemoteAddress() const {
	sockaddr_storage SockAddr;
	socklen_t        SockAddrLen = sizeof(SockAddr);
	if (getpeername(NativeSocket, (sockaddr *)&SockAddr, &SockAddrLen)) {
		return {};
	}
	return xNetAddress::Parse(&SockAddr);
}

xNetAddress xTcpConnection::GetLocalAddress() const {
	sockaddr_storage SockAddr;
	socklen_t        SockAddrLen = sizeof(SockAddr);
	if (getsockname(NativeSocket, (sockaddr *)&SockAddr, &SockAddrLen)) {
		return {};
	}
	return xNetAddress::Parse(&SockAddr);
}

void xTcpConnection::PostRequestKeepAlive() {
	PostData(KeepAliveRequestBuffer, PacketHeaderSize);
}

void xTcpConnection::PostKeepAlive() {
	PostData(KeepAliveBuffer, PacketHeaderSize);
}

X_END
