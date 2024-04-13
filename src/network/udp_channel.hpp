#pragma once

#include "./io_context.hpp"
#include "./net_address.hpp"
#include "./packet.hpp"

X_BEGIN

class xUdpChannel
	: xSocketIoReactor
	, xAbstract {
public:
	struct iListener {  // clang-format off
		virtual void OnError(xUdpChannel * ChannelPtr) {}
		virtual void OnData(xUdpChannel * ChannelPtr, void * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress) = 0;
	};  // clang-format on

public:
	X_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress, iListener * ListenerPtr);
	X_API_MEMBER void Clean();
	X_API_MEMBER void PostData(const void * DataPtr, size_t DataSize, const xNetAddress & DestiationAddress);

private:
	X_API_MEMBER bool OnIoEventInReady() override;
	X_API_MEMBER void OnIoEventError() override;

private:
	xIoContext * ICP = nullptr;
	iListener *  LP  = nullptr;

	ubyte ReadBuffer[MaxPacketSize];
};

X_END
