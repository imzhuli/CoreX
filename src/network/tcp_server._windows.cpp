#include "./tcp_server.hpp"

#include "../core/string.hpp"
#include "./socket.hpp"

#ifdef X_SYSTEM_WINDOWS
X_BEGIN

bool xTcpServer::Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress, iListener * ListenerPtr) {
    Todo();
    return false;
}

void xTcpServer::Clean() {
    Todo();
}

bool xTcpServer::OnIoEventInReady() {
    Todo();
    return false;
}

bool xTcpServer::TryListen() {
    Todo();
    return false;
}

bool xTcpServer::TryAccept(xSocket & NewConnectionSocket) {
    Todo();
    return false;
}

X_END
#endif
