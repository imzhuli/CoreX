#include "./tcp_server.hpp"

#ifdef X_SYSTEM_WINDOWS

#include "../core/string.hpp"
#include "./socket.hpp"

#include <cinttypes>

X_BEGIN

struct xPreAcceptInfo {
	int     AF              = AF_UNSPEC;
	xSocket PreAcceptSocket = InvalidSocket;
	struct {
		sockaddr_storage Local;
		ubyte            LocalAddressPadding[16];
		sockaddr_storage Remote;
		ubyte            RemoteAddressPadding[16];
		DWORD            PreAcceptReceivedLength = 0;
	} PreAcceptAddress;
};

static void PAIPDeleter(xOverlappedIoBuffer * IBP) {
	auto PAIP = reinterpret_cast<xPreAcceptInfo *>(Steal(IBP->Reader.BufferUsage.buf));
	if (PAIP->PreAcceptSocket != InvalidSocket) {
		XelCloseSocket(PAIP->PreAcceptSocket);
	}
	assert(PAIP);
	delete PAIP;
}

bool xTcpServer::Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress, iListener * ListenerPtr, bool ReuseAddress) {
	this->ICP = IoContextPtr;
	this->LP  = ListenerPtr;
	if (!xSocketIoReactor::Init()) {
		return false;
	}

	auto BaseG = xScopeGuard([this] {
		xSocketIoReactor::Clean();
		Reset(ICP);
		Reset(LP);
	});

	auto PAIP = new (std::nothrow) xPreAcceptInfo{};
	if (!PAIP) {
		return false;
	}
	IBP->Reader.BufferUsage.buf = reinterpret_cast<CHAR *>(PAIP);
	IBP->Cleaner                = &PAIPDeleter;

	// Create listening socket
	PAIP->AF     = BindAddress.IsV4() ? AF_INET : AF_INET6;  // address family
	NativeSocket = WSASocket(PAIP->AF, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (NativeSocket == InvalidSocket) {
		return false;
	}
	ResizeSendBuffer(NativeSocket, 0);
	if (ReuseAddress) {
		SetSocketReuseAddress(NativeSocket);
	}
	auto SG = xScopeGuard([this] { XelCloseSocket(Steal(NativeSocket, InvalidSocket)); });

	// Binding to local address
	sockaddr_storage AddrStorage;
	size_t           AddrLen = BindAddress.Dump(&AddrStorage);
	auto             BindRet = bind(NativeSocket, (sockaddr *)&AddrStorage, (int)AddrLen);
	if (BindRet == SOCKET_ERROR) {
		X_DEBUG_PRINTF("Failed to bind: socket=%" PRIuPTR "\n", (uintptr_t)NativeSocket);
		return false;
	}
	auto ListenRet = listen(NativeSocket, SOMAXCONN);
	if (ListenRet == SOCKET_ERROR) {
		X_DEBUG_PRINTF("Failed to listen: socket=%" PRIuPTR "\n", (uintptr_t)NativeSocket);
		return false;
	}

	// create pre_accept socket:
	if (!IoContextPtr->Add(*this)) {
		X_PFATAL("failed to register tcp server socket");
		return false;
	}

	assert(PAIP->PreAcceptSocket == InvalidSocket);
	TryPreAccept();
	Dismiss(BaseG, SG);
	return true;
}

void xTcpServer::Clean() {
	XelCloseSocket(Steal(NativeSocket, InvalidSocket));
	xSocketIoReactor::Clean();
	Reset(ICP);
	Reset(LP);
}

bool xTcpServer::OnIoEventInReady() {
	X_DEBUG_PRINTF("");
	auto PAIP = reinterpret_cast<xPreAcceptInfo *>(IBP->Reader.BufferUsage.buf);
	setsockopt(PAIP->PreAcceptSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char *)&NativeSocket, sizeof(NativeSocket));
	LP->OnNewConnection(this, Steal(PAIP->PreAcceptSocket, InvalidSocket));
	TryPreAccept();
	return true;
}

void xTcpServer::TryPreAccept() {
	auto PAIP = reinterpret_cast<xPreAcceptInfo *>(IBP->Reader.BufferUsage.buf);
	assert(PAIP->PreAcceptSocket == InvalidSocket);

	PAIP->PreAcceptSocket = WSASocket(PAIP->AF, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
	X_RUNTIME_ASSERT(PAIP->PreAcceptSocket != InvalidSocket);

	auto & Overlapped = IBP->Reader.Native.Overlapped;
	memset(&Overlapped, 0, sizeof(Overlapped));
	auto AcceptResult = AcceptEx(
		NativeSocket, PAIP->PreAcceptSocket, &PAIP->PreAcceptAddress, 0, sizeof(PAIP->PreAcceptAddress.Local) + 16, sizeof(PAIP->PreAcceptAddress.Remote) + 16,
		&PAIP->PreAcceptAddress.PreAcceptReceivedLength, &Overlapped
	);
	if (!AcceptResult && ERROR_IO_PENDING != WSAGetLastError()) {
		X_DEBUG_PRINTF("Failed to exec AcceptEx: reason=%i\n", WSAGetLastError());
		ICP->DeferError(*this);
		return;
	}
	Retain(IBP);
}

X_END
#endif
