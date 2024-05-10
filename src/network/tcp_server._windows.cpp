#include "./tcp_server.hpp"

#include "../core/string.hpp"
#include "./socket.hpp"

#ifdef X_SYSTEM_WINDOWS
X_BEGIN

bool xTcpServer::Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress, iListener * ListenerPtr) {
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

	Todo("");

	if (!IoContextPtr->Add(*this)) {
		X_PFATAL("failed to register tcp server socket");
		return false;
	}
	Dismiss(BaseG);
	return true;
}

void xTcpServer::Clean() {
	xSocketIoReactor::Clean();
	Reset(ICP);
	Reset(LP);
}

bool xTcpServer::OnIoEventInReady() {
	Todo("");
	return false;
}

bool xTcpServer::TryListen() {
	Todo("");
	return false;
}

bool xTcpServer::TryAccept(xSocket & NewConnectionSocket) {
	Todo("");
	return false;
}

X_END
#endif
