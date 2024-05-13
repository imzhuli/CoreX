#include "./socket.hpp"

X_BEGIN

void DestroySocket(xSocket && Socket) {
	XelCloseSocket(Steal(Socket, InvalidSocket));
}

X_API void SetSocketNoDelay(xSocket Socket, bool EnableNoDelay) {
	int one = EnableNoDelay ? 1 : 0;
	setsockopt(Socket, IPPROTO_TCP, TCP_NODELAY, (char *)&one, sizeof(one));
}

void ResizeSendBuffer(xSocket Socket, size_t Size) {
	setsockopt(Socket, SOL_SOCKET, SO_SNDBUF, (char *)X2P(int(Size)), sizeof(int));
}

void ResizeRecvBuffer(xSocket Socket, size_t Size) {
	setsockopt(Socket, SOL_SOCKET, SO_RCVBUF, (char *)X2P(int(Size)), sizeof(int));
}

X_END
