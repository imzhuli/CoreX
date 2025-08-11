
#pragma once
#include "../core/indexed_storage.hpp"
#include "../core/list.hpp"
#include "../network/tcp_connection.hpp"
#include "../network/tcp_server.hpp"
#include "../network/udp_channel.hpp"
#include "./message.hpp"

X_BEGIN

class xUdpService
	: xUdpChannel
	, xUdpChannel::iListener {
public:
	X_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress) { return xUdpChannel::Init(IoContextPtr, BindAddress, this, true); }
	X_API_MEMBER void Clean() { xUdpChannel::Clean(); }

	X_API_MEMBER void PostMessage(const xNetAddress & RemoteAddress, xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);

protected:
	using xUdpChannel::PostData;
	X_API_MEMBER
	virtual void OnPacket(const xNetAddress & RemoteAddress, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);

private:
	X_PRIVATE_MEMBER void OnData(xUdpChannel * ChannelPtr, ubyte * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress) override;
};

X_END
