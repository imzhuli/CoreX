#pragma once

#include "../core/core_min.hpp"
#include "../network/net_address.hpp"
#include "./io_context.hpp"

X_BEGIN

class xTcpServer : protected xSocketIoReactor {
public:
	struct iListener {
		virtual void OnNewConnection(xTcpServer * TcpServerPtr, xSocket && NativeHandle) = 0;
	};
	X_INLINE xIoContext * GetIoContextPtr() const { return ICP; }

public:
	X_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & Address, iListener * ListenerPtr, bool ReuseAddress = true);
	X_API_MEMBER void Clean();

private:
	X_PRIVATE_MEMBER void OnIoEventError() override { X_PFATAL("TcpServerError"); }
	X_PRIVATE_MEMBER bool OnIoEventInReady() override;

#ifdef X_SYSTEM_WINDOWS
	X_PRIVATE_MEMBER void TryPreAccept();
#else
	X_PRIVATE_MEMBER bool TryListen();
	X_PRIVATE_MEMBER bool TryAccept(xSocket & NewConnectionSocket);
#endif

private:
	xIoContext * ICP = nullptr;
	iListener *  LP  = nullptr;
};

X_END