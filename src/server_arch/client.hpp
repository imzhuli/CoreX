#pragma once
#include "../core/functional.hpp"
#include "../network/tcp_connection.hpp"
#include "./message.hpp"

X_BEGIN

class xClient
	: public xTcpConnection::iListener
	, xAbstract {
public:
	X_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & TargetAddress) { return Init(IoContextPtr, TargetAddress, TargetAddress.Decay()); }
	X_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & TargetAddress, const xNetAddress & BindAddress);
	X_API_MEMBER void Tick(uint64_t NowMS);
	X_API_MEMBER void Clean();

	X_INLINE bool                IsOpen() const { return Connection.IsOpen(); }
	X_INLINE uint64_t            GetTickTimeMS() const { return NowMS; }
	X_INLINE const xNetAddress & GetTargetAddress() const { return TargetAddress; }

	X_API_MEMBER void DisableKeepAliveOnTick();
	X_API_MEMBER void SetDefaultKeepAliveTimeout();
	X_API_MEMBER void SetKeepAliveTimeout(uint64_t TimeoutMS);
	X_API_MEMBER void SetMaxWriteBuffer(size_t Size);
	X_API_MEMBER void PostData(const void * DataPtr, size_t DataSize);
	X_API_MEMBER void PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);
	X_API_MEMBER void PostRequestKeepAlive();
	X_API_MEMBER void Kill();

public:
	using xOnServerConnected    = std::function<void()>;
	using xOnServerDisconnected = std::function<void()>;
	using xOnServerPacket       = std::function<bool(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize)>;

	xOnServerConnected    OnServerConnected    = Noop<>;
	xOnServerDisconnected OnServerDisconnected = Noop<>;
	xOnServerPacket       OnServerPacket       = Noop<true>;

private:  // tcp connection listener
	X_PRIVATE_MEMBER void   OnConnected(xTcpConnection * TcpConnectionPtr) override;
	X_PRIVATE_MEMBER void   OnPeerClose(xTcpConnection * TcpConnectionPtr) override;
	X_PRIVATE_MEMBER size_t OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) override;

private:
	// config
	size_t MaxWriteBufferLimitForEachConnection = 100'000'000 / sizeof(xPacketBuffer::Buffer);

	xIoContext *   IoContextPtr = {};
	xNetAddress    TargetAddress;
	xNetAddress    BindAddress;
	xTcpConnection Connection;

	uint64_t NowMS                           = 0;
	uint64_t ReconnectTimestampMS            = 0;
	uint64_t KeepAliveTimeoutMS              = 0;
	uint64_t LastKeepAliveTimestampMS        = 0;
	uint64_t LastRequestKeepAliveTimestampMS = 0;
	bool     KillConnection                  = false;
};

X_END
