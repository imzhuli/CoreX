#pragma once
#include "../core/indexed_storage.hpp"
#include "../core/list.hpp"
#include "../network/tcp_connection.hpp"
#include "../network/tcp_server.hpp"
#include "../network/udp_channel.hpp"
#include "./message.hpp"

X_BEGIN

class xService;
class xServiceClientConnection;

class xServiceClientConnectionNode : public xListNode {
public:
	static constexpr const uint64_t BEING_KILLED = (uint64_t)-1;

	X_INLINE void SetBeingKilled() { TimestampMS = BEING_KILLED; }
	X_INLINE bool IsBeingKilled() const { return TimestampMS == BEING_KILLED; }

	uint64_t TimestampMS = 0;
};

class xServiceClientConnection final
	: public xTcpConnection
	, public xServiceClientConnectionNode {
public:
	X_INLINE xIndexId          GetConnectionId() const { return ConnectionId; }
	X_INLINE void              SetUserContext(xVariable V) { UserContext = V; }
	X_INLINE xVariable &       GetUserContext() { return UserContext; }
	X_INLINE const xVariable & GetUserContext() const { return UserContext; }

private:
	xIndexId  ConnectionId = {};
	xVariable UserContext  = {};
	friend class xService;
};
using xServiceClientConnectionList = xList<xServiceClientConnectionNode>;

class xService
	: xTcpServer::iListener
	, xTcpConnection::iListener
	, xAbstract {
public:
	X_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress, bool ReusePort = false);
	X_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress, bool ReusePort, size_t MaxConnectionId);
	X_API_MEMBER void Tick();
	X_API_MEMBER void Tick(uint64_t UpdatedNowMS);
	X_API_MEMBER void Clean();

	X_INLINE uint64_t GetTickTimeMS() const { return NowMS; }

public:
	X_API_MEMBER void SetMaxWriteBuffer(size_t Size);
	X_API_MEMBER void PostData(uint64_t ConnectionId, const void * DataPtr, size_t DataSize);
	X_API_MEMBER void PostData(xServiceClientConnection & Connection, const void * DataPtr, size_t DataSize);
	X_API_MEMBER void PostMessage(uint64_t ConnectionId, xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);
	X_API_MEMBER void PostMessage(xServiceClientConnection & Connection, xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);

	X_INLINE void DeferKillConnection(xServiceClientConnection & Connection) {
		Connection.SetBeingKilled();
		ServiceConnectionKillList.GrabTail(Connection);
	}
	X_INLINE void KeepAlive(xServiceClientConnection & Connection) {
		assert(!Connection.IsBeingKilled());
		Connection.TimestampMS = NowMS;
		ServiceConnectionTimeoutList.GrabTail(Connection);
	}

protected:
	X_API_MEMBER virtual void OnTick(uint64_t NowMS);
	X_API_MEMBER virtual void OnClientConnected(xServiceClientConnection & Connection);
	X_API_MEMBER virtual void OnClientClose(xServiceClientConnection & Connection);
	X_API_MEMBER virtual bool OnClientPacket(
		xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize
	);
	X_API_MEMBER virtual void OnCleanupClientConnection(const xServiceClientConnection & Connection);

private:
	X_PRIVATE_MEMBER void   OnNewConnection(xTcpServer * TcpServerPtr, xSocket && NativeHandle) override;
	X_PRIVATE_MEMBER void   OnPeerClose(xTcpConnection * TcpConnectionPtr) override;
	X_PRIVATE_MEMBER size_t OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) override;

	X_PRIVATE_MEMBER void CleanupConnection(xServiceClientConnection & Connection);
	X_PRIVATE_MEMBER void CleanupKilledConnections();

	[[nodiscard]] X_STATIC_INLINE xServiceClientConnection & Cast(xTcpConnection & Connection) { return static_cast<xServiceClientConnection &>(Connection); };
	[[nodiscard]] X_STATIC_INLINE xServiceClientConnection & Cast(xServiceClientConnectionNode & Node) { return static_cast<xServiceClientConnection &>(Node); };

private:
	// config
	size_t MaxWriteBufferLimitForEachConnection = 100'000'000 / sizeof(xPacketBuffer::Buffer);

	//
	uint64_t                                    NowMS;
	xTcpServer                                  TcpServer;
	xIndexedStorage<xServiceClientConnection *> ConnectionIdPool;

	xServiceClientConnectionList ServiceConnectionTimeoutList;
	xServiceClientConnectionList ServiceConnectionKillList;
};

class xUdpService
	: xUdpChannel
	, xUdpChannel::iListener {
public:
	X_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress) { return xUdpChannel::Init(IoContextPtr, BindAddress, this); }
	X_API_MEMBER void Clean() { xUdpChannel::Clean(); }
	using xUdpChannel::PostData;

protected:
	X_API_MEMBER
	virtual void OnPacket(const xNetAddress & RemoteAddress, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize);

private:
	X_PRIVATE_MEMBER void OnData(xUdpChannel * ChannelPtr, ubyte * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress) override;
};

X_END
