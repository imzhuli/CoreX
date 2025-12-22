
#pragma once
#include "../core/functional.hpp"
#include "../core/indexed_storage.hpp"
#include "../core/list.hpp"
#include "../network/tcp_connection.hpp"
#include "../network/tcp_server.hpp"
#include "../network/udp_channel.hpp"
#include "./message.hpp"

X_BEGIN

class xUdpService;
class xUdpServiceChannelHandle;

class xUdpServiceChannelHandle final {
public:
	X_INLINE xUdpServiceChannelHandle() = default;

	X_API_MEMBER bool        IsValid() const;
	X_API_MEMBER xNetAddress GetRemoteAddress() const { return RemoteAddress; }
	X_API_MEMBER void        PostData(const void * DataPtr, size_t DataSize) const;
	X_API_MEMBER void        PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) const;

private:
	friend class xUdpService;
	xUdpServiceChannelHandle(xUdpService * Service, const xNetAddress & RemoteAddress)
		: Service(Service), RemoteAddress(RemoteAddress) {  //
	}
	xUdpService * Service       = nullptr;
	xNetAddress   RemoteAddress = {};
};

class xUdpService final
	: private xUdpChannel
	, private xUdpChannel::iListener {
public:
	bool Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress) { return xUdpChannel::Init(IoContextPtr, BindAddress, this, true); }
	using xUdpChannel::Clean;
	using xUdpChannel::PostData;
	X_API_MEMBER void PostMessage(const xNetAddress & RemoteAddress, xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);

	using xOnPacketCallback            = std::function<void(const xUdpServiceChannelHandle &, xPacketCommandId, xPacketRequestId, ubyte *, size_t)>;
	xOnPacketCallback OnPacketCallback = Noop<>;

private:
	X_PRIVATE_MEMBER void OnData(xUdpChannel * ChannelPtr, ubyte * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress) override;
};

X_END
