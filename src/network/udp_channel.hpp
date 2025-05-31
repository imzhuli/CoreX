#pragma once

#include "./io_context.hpp"
#include "./net_address.hpp"
#include "./packet.hpp"

X_BEGIN

class xUdpChannel
	: xSocketIoReactor
	, xAbstract {
public:
	struct iListener {
		virtual void OnError(xUdpChannel * ChannelPtr) {}
		virtual void OnData(xUdpChannel * ChannelPtr, ubyte * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress) = 0;
	};

public:
	X_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress, iListener * ListenerPtr, bool ReuseAddress = false);
	X_API_MEMBER void Clean();
	X_API_MEMBER void PostData(const void * DataPtr, size_t DataSize, const xNetAddress & DestiationAddress);
	X_API_MEMBER void PostRequestKeepAlive(const xNetAddress & DestiationAddress);
	X_API_MEMBER void PostKeepAlive(const xNetAddress & DestiationAddress);

	X_INLINE const xNetAddress & GetBindAddress() const { return ActualBindAddress; }

private:
	X_PRIVATE_MEMBER bool OnIoEventInReady() override;
	X_PRIVATE_MEMBER void OnIoEventError() override;

	X_PRIVATE_MEMBER xNetAddress GetLocalAddress() const;

#ifdef X_SYSTEM_WINDOWS
	X_PRIVATE_MEMBER void AsyncAcquireInput();
#endif

private:
	xIoContext * ICP               = nullptr;
	iListener *  LP                = nullptr;
	xNetAddress  ActualBindAddress = {};
};

X_END
