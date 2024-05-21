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
	X_INLINE xIoContext * GetIoContextPtr() const {
		return ICP;
	}

public:  // clang-format off
	X_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & Address, iListener * ListenerPtr);
	X_API_MEMBER void Clean();


private:  // clang-format off
	X_API_MEMBER void OnIoEventError() override { X_PFATAL("TcpServerError"); }
	X_API_MEMBER bool OnIoEventInReady() override;

#ifdef X_SYSTEM_WINDOWS
	X_API_MEMBER void TryPreAccept();
#else
	X_API_MEMBER bool TryListen();
	X_API_MEMBER bool TryAccept(xSocket & NewConnectionSocket);
#endif

	// clang-format on
private:
	xIoContext * ICP = nullptr;
	iListener *  LP  = nullptr;
};

X_END