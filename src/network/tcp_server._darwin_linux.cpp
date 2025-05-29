#include "./tcp_server.hpp"

#include "./socket.hpp"
#include "./tcp_connection.hpp"

#if defined(X_SYSTEM_LINUX) || defined(X_SYSTEM_DARWIN)

X_BEGIN

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

	if (!CreateNonBlockingTcpSocket(NativeSocket, BindAddress, ReuseAddress)) {
		return false;
	}
	auto SG = xScopeGuard([this] { DestroySocket(std::move(NativeSocket)); });

	if (!TryListen()) {
		return false;
	}

	if (!IoContextPtr->Add(*this)) {
		X_PFATAL("failed to register tcp server socket");
		return false;
	}

	Dismiss(BaseG, SG);
	return true;
}

void xTcpServer::Clean() {
	this->ICP->Remove(*this);
	DestroySocket(std::move(NativeSocket));
	xSocketIoReactor::Clean();
	Reset(ICP);
	Reset(LP);
}

bool xTcpServer::OnIoEventInReady() {
	auto NewSocket = InvalidSocket;
	while (TryAccept(NewSocket)) {
		if (NewSocket != InvalidSocket) {
			LP->OnNewConnection(this, std::move(NewSocket));
		}
	}
	return true;
}

bool xTcpServer::TryListen() {
	setsockopt(NativeSocket, SOL_SOCKET, SO_SNDBUF, (char *)X2P(int(0)), sizeof(int));
	auto ListenRet = listen(NativeSocket, SOMAXCONN);
	if (ListenRet == -1) {
		X_DEBUG_PRINTF("failed to listen on address");
		return false;
	}
	return true;
}

bool xTcpServer::TryAccept(xSocket & NewConnectionSocket) {
	sockaddr_storage SockAddr    = {};
	socklen_t        SockAddrLen = sizeof(SockAddr);

	NewConnectionSocket = accept(NativeSocket, (struct sockaddr *)&SockAddr, &SockAddrLen);
	if (NewConnectionSocket == InvalidSocket) {
		auto Error = errno;
		if (Error == EAGAIN) {
			return false;  // break the outer loop
		}
		// note: still need to return true, so that further queued connection request could be processed
		X_DEBUG_PRINTF("failed to accept new connection: %s", strerror(Error));
	}
	return true;
}

X_END
#endif
