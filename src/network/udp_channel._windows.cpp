#include "./udp_channel.hpp"

#include "../core/string.hpp"
#include "./socket.hpp"

#ifdef X_SYSTEM_WINDOWS
X_BEGIN

bool xUdpChannel::Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress, iListener * ListenerPtr) {
    Todo();
    return false;
}

void xUdpChannel::Clean() {
    Todo();
}

xNetAddress xUdpChannel::GetLocalAddress() const {
    Todo();
	sockaddr_storage SockAddr;
	socklen_t        SockAddrLen = sizeof(SockAddr);
	if (getsockname(NativeSocket, (sockaddr *)&SockAddr, &SockAddrLen)) {
		return {};
	}
	return xNetAddress::Parse(&SockAddr);
}

void xUdpChannel::PostData(const void * DataPtr, size_t DataSize, const xNetAddress & Address) {
    Todo();
	sockaddr_storage AddrStorage = {};
	size_t           AddrLen     = Address.Dump(&AddrStorage);
	auto             SendResult  = sendto(NativeSocket, (const char *)DataPtr, DataSize, 0, (const sockaddr *)&AddrStorage, (socklen_t)AddrLen);
	if (SendResult == -1) {
		auto Error = errno;
		X_DEBUG_PRINTF("Udp send error: code=%i, description=%s", Error, strerror(Error));
		Touch(Error);
	}
}

bool xUdpChannel::OnIoEventInReady() {
    Todo();
    return false;
}

void xUdpChannel::OnIoEventError() {
	LP->OnError(this);
}

X_END
#endif
