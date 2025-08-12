
#pragma once
#include "../core/indexed_storage.hpp"
#include "../core/list.hpp"
#include "../network/tcp_connection.hpp"
#include "../network/tcp_server.hpp"
#include "../network/udp_channel.hpp"
#include "./message.hpp"

#include <functional>

X_BEGIN

class xUdpService;
class xUdpServiceChannelHandle;

class xUdpServiceChannelHandle final {
public:
	X_API_MEMBER void PostMessage(const xNetAddress & RemoteAddress, xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) const;

private:
	friend class xUdpService;
	xUdpServiceChannelHandle(xUdpService * Service)
		: Service(Service) {  //
	}
	xUdpService * Service = nullptr;
};

class xUdpService final
	: xUdpChannel
	, xUdpChannel::iListener {
public:
	X_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress) { return xUdpChannel::Init(IoContextPtr, BindAddress, this, true); }
	X_API_MEMBER void Clean() { xUdpChannel::Clean(); }
	X_API_MEMBER void PostMessage(const xNetAddress & RemoteAddress, xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);

	using xOnPacketCallback            = std::function<void(const xUdpServiceChannelHandle &, const xNetAddress &, xPacketCommandId, xPacketRequestId, ubyte *, size_t)>;
	xOnPacketCallback OnPacketCallback = Ignore<const xUdpServiceChannelHandle &, const xNetAddress &, xPacketCommandId, xPacketRequestId, ubyte *, size_t>;

private:
	X_PRIVATE_MEMBER void OnData(xUdpChannel * ChannelPtr, ubyte * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress) override;
};

X_END
