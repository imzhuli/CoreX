#include "./socket.hpp"
#ifdef X_SYSTEM_LINUX

#include <fcntl.h>

#include <cinttypes>

// linux < 3.9
#ifndef SO_REUSEPORT
#define SO_REUSEPORT SO_REUSEADDR
#endif

// linux > 3.9 or bsd
#define X_ENABLE_REUSEPORT SO_REUSEPORT
#if defined(SO_REUSEPORT_LB)
#undef X_ENABLE_REUSEPORT
#define X_ENABLE_REUSEPORT SO_REUSEPORT_LB
#endif

X_BEGIN

bool CreateNonBlockingTcpSocket(xSocket & Socket, const xNetAddress & BindAddress, bool ReuseAddress) {
	auto   AFType      = BindAddress.GetAddressFamily();
	auto   AddrStorage = sockaddr_storage{};
	size_t AddrLen     = BindAddress.Dump(&AddrStorage);

	// create socket & set non-blocking
	Socket = socket(AFType, SOCK_STREAM, 0);
	if (Socket == InvalidSocket) {
		X_DEBUG_PRINTF("Failed to create socket\n");
		return false;
	}
	SetSocketNonBlocking(Socket);
	if (ReuseAddress) {
		SetSocketReuseAddress(Socket);
	}

	// bind address
	auto BindRet = bind(Socket, (sockaddr *)&AddrStorage, (int)AddrLen);
	if (BindRet == -1) {
		X_DEBUG_PRINTF("failed bind: socket=%" PRIuPTR ", error=%s, address=%s", (uintptr_t)Socket, strerror(errno), BindAddress.ToString().c_str());
		XelCloseSocket(Steal(Socket, InvalidSocket));
		return false;
	}

	// done
	return true;
}

bool CreateNonBlockingUdpSocket(xSocket & Socket, const xNetAddress & BindAddress, bool ReuseAddress) {
	auto   AFType      = BindAddress.GetAddressFamily();
	auto   AddrStorage = sockaddr_storage{};
	size_t AddrLen     = BindAddress.Dump(&AddrStorage);

	// create socket & set non-blocking
	Socket = socket(AFType, SOCK_DGRAM, 0);
	if (Socket == InvalidSocket) {
		X_DEBUG_PRINTF("Failed to create socket\n");
		return false;
	}
	SetSocketNonBlocking(Socket);
	if (ReuseAddress) {
		SetSocketReuseAddress(Socket);
	}

	// bind address
	auto BindRet = bind(Socket, (sockaddr *)&AddrStorage, (int)AddrLen);
	if (BindRet == -1) {
		X_DEBUG_PRINTF("failed bind: socket=%" PRIuPTR ", error=%s, address=%s", (uintptr_t)Socket, strerror(errno), BindAddress.ToString().c_str());
		XelCloseSocket(Steal(Socket, InvalidSocket));
		return false;
	}

	// done
	return true;
}

void SetSocketNonBlocking(xSocket Socket) {
	int flags = fcntl(Socket, F_GETFL);
	fcntl(Socket, F_SETFL, flags | O_NONBLOCK);
}

void SetSocketReuseAddress(xSocket Socket) {
	setsockopt(Socket, SOL_SOCKET, X_ENABLE_REUSEPORT, (char *)X2P(int(1)), sizeof(int));
}

X_END
#endif
