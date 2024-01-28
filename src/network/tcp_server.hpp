#pragma once

#include "../core/core_min.hpp"
#include "./io_context.hpp"
#include "./packet.hpp"
#include "./tcp_connection.hpp"

#include <atomic>

X_BEGIN

class xTcpServer : protected iBufferedIoReactor {
public:
	struct iListener {
		virtual void OnNewConnection(xTcpServer * TcpServerPtr, xSocket && NativeHandle) = 0;
	};

public:
	X_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & Address, iListener * ListenerPtr, bool ReusePort = false);
	X_API_MEMBER void Clean();

	X_INLINE xIoContext * GetIoContextPtr() const {
		return _IoContextPtr;
	}

private:
	X_API_MEMBER void DoAccept();
	X_API_MEMBER void OnAccept(xSocket NativeHandle);
	X_API_MEMBER void OnIoEventError() override {
		Fatal("TcpServerError");
	}

private:
	xIoContext *          _IoContextPtr;
	xSocket _ListenSocket X_DEBUG_INIT(InvalidSocket);
	iListener *           _ListenerPtr;

#if defined(X_SYSTEM_DARWIN)
	int               _AF;  // address family
	X_API_MEMBER void OnIoEventInReady() override;
#endif

#if defined(X_SYSTEM_LINUX)
	int               _AF;  // address family
	X_API_MEMBER void OnIoEventInReady() override;
#endif

#if defined(X_SYSTEM_WINDOWS)
	ADDRESS_FAMILY           _AF;
	xSocket _PreAcceptSocket X_DEBUG_INIT(InvalidSocket);
	struct {
		sockaddr_storage Local;
		ubyte            _LocalAddressPadding[16];
		sockaddr_storage Remote;
		ubyte            _RemoteAddressPadding[16];
		DWORD            _PreAcceptReceivedLength = 0;
	} _PreAcceptAddress;

	X_API_MEMBER void TryPreAccept();
	X_API_MEMBER void OnIoEventInReady() override;
#endif
};

X_END