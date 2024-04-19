#include "./udp_channel.hpp"

#include "../core/string.hpp"
#include "./socket.hpp"

#ifdef X_SYSTEM_WINDOWS
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
	ICP                     = IoContextPtr;
	LP                      = ListenerPtr;
	return true;
}

void xUdpChannel::Clean() {
	ICP->Remove(*this);
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

void xUdpChannel::AsyncAcquireInput() {
	Todo();
}

void xUdpChannel::PostData(const void * DataPtr, size_t DataSize, const xNetAddress & Address) {
	sockaddr_storage AddrStorage = {};
	size_t           AddrLen     = Address.Dump(&AddrStorage);
	auto             SendResult  = sendto(NativeSocket, (const char *)DataPtr, DataSize, 0, (const sockaddr *)&AddrStorage, (socklen_t)AddrLen);
	if (SendResult == -1) {
		auto Error = errno;
		Touch(Error);
		X_DEBUG_PRINTF("Udp send error: code=%i, description=%s", Error, strerror(Error));
	}
}

bool xUdpChannel::OnIoEventInReady() {
	Todo();

	void AsyncAcquireInput();
	return true;
}

void xUdpChannel::OnIoEventError() {
	LP->OnError(this);
}

X_END
#endif
