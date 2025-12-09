#pragma once
#include "../core/core_time.hpp"
#include "../core/functional.hpp"
#include "../core/indexed_storage.hpp"
#include "../network/tcp_connection.hpp"
#include "./message.hpp"

X_BEGIN

class xClientPool;
class xClientPoolConnection;
class xClientPoolConnectionHandle;

struct xClientPoolConnectionAvailableNode : xListNode {};
struct xClientPoolConnectionTimeoutNode : xListNode {
	union {
		uint64_t InitTimestampMS;
		uint64_t LastRequestKeepAliveTimestampMS;
		uint64_t LastKeepAliveTimestampMS;
	};
};

struct xClientPoolConnectionUserContext {
	xVariable UserContext   = {};
	xVariable UserContextEx = {};
};

class xClientPoolConnection
	: public xClientPoolConnectionUserContext
	, private xTcpConnection
	, private xClientPoolConnectionAvailableNode
	, private xClientPoolConnectionTimeoutNode {
	friend class xClientPoolConnectionHandle;
	friend class xClientPool;

private:
	X_INLINE xClientPool *       GetOwner() const { return Owner; }
	X_INLINE uint64_t            GetConnectionId() const { return ConnectionId; }
	X_INLINE const xNetAddress & GetTargetAddress() const { return TargetAddress; }

	X_MEMBER bool PostData(const void * DataPtr, size_t DataSize);
	X_MEMBER bool PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);

private:
	xClientPool * Owner;
	uint64_t      ConnectionId;
	xNetAddress   TargetAddress;
	bool          ReleaseMark = false;
};

class xClientPoolConnectionHandle final {
public:
	X_API_MEMBER bool      IsValid() const;
	X_INLINE xClientPool * GetOwner() const { return Owner; }
	X_INLINE uint64_t      GetConnectionId() const { return ConnectionId; }
	X_INLINE xNetAddress   GetTargetAddress() const { return Connection->GetTargetAddress(); }
	X_INLINE bool          PostData(const void * DataPtr, size_t DataSize) const { return Connection->PostData(DataPtr, DataSize); }
	X_INLINE bool          PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) const { return Connection->PostMessage(CmdId, RequestId, Message); }
	X_INLINE auto          operator->() const { return (xClientPoolConnectionUserContext *)Connection; }

public:
	X_API_MEMBER xClientPoolConnectionHandle(xClientPool * Owner, uint64_t ConnectionId);

private:
	friend class xClientPool;
	xClientPoolConnectionHandle(xClientPool * Owner, xClientPoolConnection * Connection)
		: Owner(Owner), Connection(Connection), ConnectionId(Connection->ConnectionId) {
		Pass();
	}

private:
	xClientPool * const           Owner        = nullptr;  // MUST be valid
	xClientPoolConnection * const Connection   = nullptr;  // valid in callbacks,
	uint64_t const                ConnectionId = 0;        // always checked by owner, use this for safety
};

class xClientPool
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

	X_INLINE uint64_t                    GetTickTimeMS() const { return Ticker(); }
	X_INLINE xClientPoolConnectionHandle GetConnectionHandle(uint64_t ConnectionId) { return { this, ConnectionId }; }

	using xOnTick            = std::function<void(uint64_t NowMS)>;
	using xOnTargetConnected = std::function<void(const xClientPoolConnectionHandle & CC)>;
	using xOnTargetClose     = std::function<void(const xClientPoolConnectionHandle & CC)>;
	using xOnTargetClean     = std::function<void(const xClientPoolConnectionHandle & CC)>;
	using xOnTargetPacket    = std::function<bool(const xClientPoolConnectionHandle & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize)>;

	xOnTick            OnTick            = Noop<>;
	xOnTargetConnected OnTargetConnected = Noop<>;
	xOnTargetClose     OnTargetClose     = Noop<>;
	xOnTargetClean     OnTargetClean     = Noop<>;
	xOnTargetPacket    OnTargetPacket    = Noop<true>;

private:
	friend class xClientPoolConnection;
	friend class xClientPoolConnectionHandle;

	X_API_MEMBER xClientPoolConnection * GetConnection(uint64_t ConnectionId);

private:
	X_PRIVATE_MEMBER void CheckTimeoutConnections();
	X_PRIVATE_MEMBER void ReleaseConnections();
	X_PRIVATE_MEMBER void DoRequestKeepAlive();
	X_PRIVATE_MEMBER void DoAutoReconnect();
	X_PRIVATE_MEMBER void DoKeepAlive(xClientPoolConnection * PC);

	X_PRIVATE_MEMBER void   OnConnected(xTcpConnection * TcpConnectionPtr) override;
	X_PRIVATE_MEMBER void   OnPeerClose(xTcpConnection * TcpConnectionPtr) override;
	X_PRIVATE_MEMBER size_t OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) override;

private:
	xIoContext *                           ICP = nullptr;
	xIndexedStorage<xClientPoolConnection> ConnectionPool;
	xTicker                                Ticker;

	xList<xClientPoolConnectionAvailableNode> EstablishedConnectionList;
	xList<xClientPoolConnectionTimeoutNode>   ReleaseConnectionList;
	xList<xClientPoolConnectionTimeoutNode>   KillConnectionList;
	xList<xClientPoolConnectionTimeoutNode>   AutoConnectionList;
	xList<xClientPoolConnectionTimeoutNode>   WaitForConnectionEstablishmentQueue;
	xList<xClientPoolConnectionTimeoutNode>   RequestKeepAliveQueue;
	xList<xClientPoolConnectionTimeoutNode>   WaitForKeepAliveQueue;
};

X_END
