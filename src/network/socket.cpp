#include "./socket.hpp"

X_BEGIN

void DestroySocket(xSocket && Socket) {
	XelCloseSocket(Steal(Socket, InvalidSocket));
}

X_API void SetSocketNoDelay(xSocket Socket, bool EnableNoDelay) {
	int one = EnableNoDelay ? 1 : 0;
	setsockopt(Socket, IPPROTO_TCP, TCP_NODELAY, (char *)&one, sizeof(one));
}

X_END
