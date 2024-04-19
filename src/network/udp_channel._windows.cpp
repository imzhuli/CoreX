#include "./udp_channel.hpp"

#include "../core/string.hpp"
#include "./socket.hpp"

#ifdef X_SYSTEM_WINDOWS
X_BEGIN

bool xUdpChannel::Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress, iListener * ListenerPtr) {
	if (!xSocketIoReactor::Init()) {
		return false;
	}
	auto BaseG = xScopeGuard([this] { xSocketIoReactor::Clean(); });

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
	auto & BU = IBP->ReadBufferUsage;
	BU.buf    = (CHAR *)IBP->ReadBuffer;
	BU.len    = (ULONG)sizeof(IBP->ReadBuffer);
	memset(&IBP->ReadObject, 0, sizeof(IBP->ReadObject));
	memset(&IBP->ReadFromAddress, 0, sizeof(IBP->ReadFromAddress));
	IBP->ReadFromAddressLength = sizeof(IBP->ReadFromAddress);
	if (WSARecvFrom(NativeSocket, &BU, 1, nullptr, X2P(DWORD(0)), (sockaddr *)&IBP->ReadFromAddress, &IBP->ReadFromAddressLength, &IBP->ReadObject, nullptr)) {
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
	auto RemoteAddress = xNetAddress::Parse((sockaddr *)&IBP->ReadFromAddress);
	LP->OnData(this, IBP->ReadBuffer, IBP->ReadDataSize, RemoteAddress);
	AsyncAcquireInput();
	return true;
}

void xUdpChannel::OnIoEventError() {
	LP->OnError(this);
}

X_END
#endif
