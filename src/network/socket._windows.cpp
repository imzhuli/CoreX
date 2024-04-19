#include "./socket.hpp"

#include <cinttypes>

#ifdef X_SYSTEM_WINDOWS
X_BEGIN

bool CreateNonBlockingTcpSocket(xSocket & Socket, const xNetAddress & BindAddress) {
	auto   AFType      = BindAddress.GetAddressFamily();
	auto   AddrStorage = sockaddr_storage{};
	size_t AddrLen     = BindAddress.Dump(&AddrStorage);

	// create socket & set non-blocking
    Socket = WSASocket(AFType, SOCK_STREAM, IPPROTO_IP, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (Socket == InvalidSocket) {
		X_DEBUG_PRINTF("Failed to create socket\n");
		return false;
	}
	SetSocketNonBlocking(Socket);
	SetSocketReuseAddress(Socket);

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

bool CreateNonBlockingUdpSocket(xSocket & Socket, const xNetAddress & BindAddress) {
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
	SetSocketReuseAddress(Socket);

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
    ioctlsocket(Socket, FIONBIO, X2P((u_long)1));
}

void SetSocketReuseAddress(xSocket Socket) {
    setsockopt(Socket, SOL_SOCKET, SO_REUSEADDR, (char *)X2P((int)1), sizeof(int));
}

X_END
#endif
