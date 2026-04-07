#pragma once
#include "../core/core_time.hpp"
#include "../core/functional.hpp"
#include "../core/indexed_storage.hpp"
#include "../network/tcp_connection.hpp"
#include "./message.hpp"

X_BEGIN

class xTcpClientPool;
class xTcpClientPoolConnection;
class xTcpClientPoolConnectionHandle;

struct xTcpClientPoolConnectionAvailableNode : xListNode {};
struct xTcpClientPoolConnectionTimeoutNode : xListNode {
	union {
		uint64_t InitTimestampMS;
		uint64_t LastRequestKeepAliveTimestampMS;
		uint64_t LastKeepAliveTimestampMS;
	};
};

struct xTcpClientPoolConnectionUserContext {
	xVariable UserContext   = {};
	xVariable UserContextEx = {};
};

class xTcpClientPoolConnection
	: public xTcpClientPoolConnectionUserContext
	, private xTcpConnection
	, private xTcpClientPoolConnectionAvailableNode
	, private xTcpClientPoolConnectionTimeoutNode {
	friend class xTcpClientPoolConnectionHandle;
	friend class xTcpClientPool;

private:
	X_INLINE xTcpClientPool *    GetOwner() const { return Owner; }
	X_INLINE uint64_t            GetConnectionId() const { return ConnectionId; }
	X_INLINE const xNetAddress & GetTargetAddress() const { return TargetAddress; }

	X_MEMBER bool PostData(const void * DataPtr, size_t DataSize);
	X_MEMBER bool PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);

private:
	xTcpClientPool * Owner;
	uint64_t         ConnectionId;
	xNetAddress      TargetAddress;
	bool             ReleaseMark = false;
};

class xTcpClientPoolConnectionHandle final {
public:
	X_API_MEMBER bool         IsValid() const;
	X_INLINE xTcpClientPool * GetOwner() const { return Owner; }
	X_INLINE uint64_t         GetConnectionId() const { return ConnectionId; }
	X_INLINE xNetAddress      GetTargetAddress() const { return Connection->GetTargetAddress(); }
	X_INLINE bool             PostData(const void * DataPtr, size_t DataSize) const { return Connection->PostData(DataPtr, DataSize); }
	X_INLINE bool             PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) const { return Connection->PostMessage(CmdId, RequestId, Message); }
	X_INLINE auto             operator->() const { return (xTcpClientPoolConnectionUserContext *)Connection; }

public:
	X_INLINE xTcpClientPoolConnectionHandle() = default;

private:
	friend class xTcpClientPool;
	X_INLINE xTcpClientPoolConnectionHandle(xTcpClientPool * Owner, xTcpClientPoolConnection * Connection)
		: Owner(Owner), Connection(Connection), ConnectionId(Connection -> ConnectionId) {
		Pass();
	}
	X_API_MEMBER xTcpClientPoolConnectionHandle(xTcpClientPool * Owner, uint64_t ConnectionId);

private:
	xTcpClientPool *           Owner        = nullptr;  // MUST be valid
	xTcpClientPoolConnection * Connection   = nullptr;  // valid in callbacks,
	uint64_t                   ConnectionId = 0;        // always checked by owner, use this for safety
};

class xTcpClientPool
	: xTcpConnection::iListener
	, xAbstract {

public:
	X_API_MEMBER bool Init(xIoContext * ICP, size_t MaxConnectionCount = 1024);
	X_API_MEMBER void Clean();
	X_API_MEMBER void Tick();
	X_API_MEMBER void Tick(uint64_t NowMS);

	X_API_MEMBER uint64_t AddServer(const xNetAddress & Address);
	X_API_MEMBER void     RemoveServer(uint64_t ConnectionId);

	X_API_MEMBER bool PostData(const void * DataPtr, size_t DataSize);
	X_API_MEMBER bool PostData(uint64_t ConnectionId, const void * DataPtr, size_t DataSize);
	X_API_MEMBER bool PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);
	X_API_MEMBER bool PostMessage(uint64_t ConnectionId, xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);
	X_API_MEMBER void KillAllConnections();

	X_INLINE uint64_t                       GetTickTimeMS() const { return Ticker(); }
	X_INLINE xTcpClientPoolConnectionHandle GetConnectionHandle(uint64_t ConnectionId) { return { this, ConnectionId }; }

	using xOnTargetConnected = std::function<void(const xTcpClientPoolConnectionHandle & CC)>;
	using xOnTargetClose     = std::function<void(const xTcpClientPoolConnectionHandle & CC)>;
	using xOnTargetClean     = std::function<void(const xTcpClientPoolConnectionHandle & CC)>;
	using xOnTargetPacket    = std::function<bool(const xTcpClientPoolConnectionHandle & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize)>;

	xOnTargetConnected OnTargetConnected = Noop<>;
	xOnTargetClose     OnTargetClose     = Noop<>;
	xOnTargetClean     OnTargetClean     = Noop<>;
	xOnTargetPacket    OnTargetPacket    = Noop<true>;

private:
	friend class xTcpClientPoolConnection;
	friend class xTcpClientPoolConnectionHandle;

	X_API_MEMBER xTcpClientPoolConnection * GetConnection(uint64_t ConnectionId);

private:
	X_PRIVATE_MEMBER void CheckTimeoutConnections();
	X_PRIVATE_MEMBER void ReleaseConnections();
	X_PRIVATE_MEMBER void DoRequestKeepAlive();
	X_PRIVATE_MEMBER void DoAutoReconnect();
	X_PRIVATE_MEMBER void DoKeepAlive(xTcpClientPoolConnection * PC);

	X_PRIVATE_MEMBER void   OnConnected(xTcpConnection * TcpConnectionPtr) override;
	X_PRIVATE_MEMBER void   OnPeerClose(xTcpConnection * TcpConnectionPtr) override;
	X_PRIVATE_MEMBER size_t OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) override;

private:
	xIoContext *                              ICP = nullptr;
	xIndexedStorage<xTcpClientPoolConnection> ConnectionPool;
	xTicker                                   Ticker;

	xList<xTcpClientPoolConnectionAvailableNode> EstablishedConnectionList;
	xList<xTcpClientPoolConnectionTimeoutNode>   ReleaseConnectionList;
	xList<xTcpClientPoolConnectionTimeoutNode>   KillConnectionList;
	xList<xTcpClientPoolConnectionTimeoutNode>   AutoConnectionList;
	xList<xTcpClientPoolConnectionTimeoutNode>   WaitForConnectionEstablishmentQueue;
	xList<xTcpClientPoolConnectionTimeoutNode>   RequestKeepAliveQueue;
	xList<xTcpClientPoolConnectionTimeoutNode>   WaitForKeepAliveQueue;
};

X_END
