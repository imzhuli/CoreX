// clang-format off

#include "./udp_channel.hpp"

#include "../core/string.hpp"
#include "./socket.hpp"

#ifdef X_SYSTEM_WINDOWS
X_BEGIN

bool xUdpChannel::Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress, iListener * ListenerPtr, bool ReuseAddress) {
	if (!xSocketIoReactor::Init()) {
		return false;
	}
	auto BaseG = xScopeGuard([this] { xSocketIoReactor::Clean(); });

	if (!CreateNonBlockingUdpSocket(NativeSocket, BindAddress, ReuseAddress)) {
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
	AsyncAcquireInput();
	Dismiss(BaseG);
	return true;
}

void xUdpChannel::Clean() {
	ICP->Remove(*this);
	DestroySocket(std::move(NativeSocket));
	Reset(ActualBindAddress);
	Reset(LP);
	Reset(ICP);
	xSocketIoReactor::Clean();
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
	IBP->ReadDataSize = 0;  // reset for every udp packet
	memset(&IBP->Reader.Native.Overlapped, 0, sizeof(IBP->Reader.Native.Overlapped));
	memset(&IBP->Reader.FromAddress, 0, sizeof(IBP->Reader.FromAddress));
	IBP->Reader.FromAddressLength = sizeof(IBP->Reader.FromAddress);
	IBP->Reader.BufferUsage.buf   = (CHAR *)IBP->ReadBuffer;
	IBP->Reader.BufferUsage.len   = (ULONG)sizeof(IBP->ReadBuffer);
	if (WSARecvFrom(
			NativeSocket, &IBP->Reader.BufferUsage, 1, nullptr, XP(DWORD(0)),
			(sockaddr *)&IBP->Reader.FromAddress, &IBP->Reader.FromAddressLength,
			&IBP->Reader.Native.Overlapped,
			nullptr
		)) {
		auto ErrorCode = WSAGetLastError();
		if (ErrorCode != WSA_IO_PENDING) {
			X_DEBUG_PRINTF("WSARecvFrom ErrorCode: %u\n", ErrorCode);
			ICP->DeferError(*this);
			return;
		}
	}
	Retain(IBP);
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
	auto RemoteAddress = xNetAddress::Parse((sockaddr *)&IBP->Reader.FromAddress);
	LP->OnData(this, IBP->ReadBuffer, IBP->ReadDataSize, RemoteAddress);
	AsyncAcquireInput();
	return true;
}

void xUdpChannel::OnIoEventError() {
	LP->OnError(this);
}

X_END
#endif
