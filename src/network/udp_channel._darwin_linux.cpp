#include "./udp_channel.hpp"

#include "./socket.hpp"

#if defined(X_SYSTEM_LINUX) || defined(X_SYSTEM_DARWIN)

#include <fcntl.h>

X_BEGIN

bool xUdpChannel::Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress, iListener * ListenerPtr) {
	if (!CreateNonBlockingUdpSocket(NativeSocket, BindAddress)) {
		return false;
	}
	if (!IoContextPtr->Add(*this)) {
		X_DEBUG_PRINTF("Failed to add to listener");
		DestroySocket(std::move(NativeSocket));
		return false;
	}
	this->ActualBindAddress = GetLocalAddress();
	this->ICP               = IoContextPtr;
	this->LP                = ListenerPtr;
	return true;
}

void xUdpChannel::Clean() {
	DestroySocket(std::move(NativeSocket));
	Reset(ActualBindAddress);
	Reset(LP);
	Reset(ICP);
}

xNetAddress xUdpChannel::GetLocalAddress() const {
	sockaddr_storage SockAddr;
	socklen_t        SockAddrLen = sizeof(SockAddr);
	if (getsockname(NativeSocket, (sockaddr *)&SockAddr, &SockAddrLen)) {
		return {};
	}
	return xNetAddress::Parse(&SockAddr);
}

void xUdpChannel::PostData(const void * DataPtr, size_t DataSize, const xNetAddress & Address) {
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
	// xUdpChannel * ChannelPtr, void * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress)
	while (true) {
		sockaddr_storage SockAddr;
		socklen_t        SockAddrLen = sizeof(SockAddr);
		int              ReadSize    = recvfrom(NativeSocket, ReadBuffer, sizeof(ReadBuffer), 0, (sockaddr *)&SockAddr, &SockAddrLen);
		if (ReadSize == -1) {
			auto Error = errno;
			if (Error == EMSGSIZE) {
				continue;
			}
			if (Error == EAGAIN) {
				return true;
			}
			return false;
		}
		xNetAddress RemoteAddress = xNetAddress::Parse((const sockaddr *)&SockAddr);
		LP->OnData(this, ReadBuffer, (size_t)ReadSize, RemoteAddress);
	}
	return true;
}

void xUdpChannel::OnIoEventError() {
	LP->OnError(this);
}

X_END

#endif