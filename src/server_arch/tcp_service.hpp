
#pragma once
#include "../core/core_time.hpp"
#include "../core/functional.hpp"
#include "../core/indexed_storage.hpp"
#include "../core/list.hpp"
#include "../network/tcp_connection.hpp"
#include "../network/tcp_server.hpp"
#include "./message.hpp"

X_BEGIN

class xTcpService;
class xTcpServiceClientConnection;
class xTcpServiceClientConnectionHandle;

class xTcpServiceClientConnectionNode : public xListNode {
	// flags
	static constexpr const uint64_t BEING_KILLED = (uint64_t)0x01;

	//
	X_INLINE void ClearFlag(uint64_t Flag) { Flags &= ~Flag; }
	X_INLINE void SetFlag(uint64_t Flag) { Flags |= Flag; }
	X_INLINE bool GetFlag(uint64_t Flag) const { return Flag == (Flags & Flag); }

	X_INLINE void SetFlag_Killed() { SetFlag(BEING_KILLED); }
	X_INLINE bool HasFlag_Killed() const { return GetFlag(BEING_KILLED); }

	friend class xTcpService;
	uint64_t Flags       = 0;
	uint64_t TimestampMS = 0;
};
using xTcpServiceClientConnectionList = xList<xTcpServiceClientConnectionNode>;

struct xTcpServiceClientConnectionUserContext {
	xVariable UserContext   = {};
	xVariable UserContextEx = {};
};

class xTcpServiceClientConnection final
	: public xTcpServiceClientConnectionUserContext
	, private xTcpConnection
	, private xTcpServiceClientConnectionNode {

private:
	friend class xTcpService;
	friend class xTcpServiceClientConnectionHandle;

	X_MEMBER bool PostData(const void * DataPtr, size_t DataSize);
	X_MEMBER bool PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);

private:
	uint64_t ConnectionId = {};
};

class xTcpServiceClientConnectionHandle final {
public:
	X_API_MEMBER bool      IsValid() const;
	X_API_MEMBER void      Kill() const;
	X_INLINE xTcpService * GetOwner() const { return Owner; }
	X_INLINE uint64_t      GetConnectionId() const { return ConnectionId; }
	X_INLINE xNetAddress   GetLocalAddress() const { return Connection->GetLocalAddress(); }
	X_INLINE xNetAddress   GetRemoteAddress() const { return Connection->GetRemoteAddress(); }
	X_INLINE bool          PostData(const void * DataPtr, size_t DataSize) const { return Connection->PostData(DataPtr, DataSize); }
	X_INLINE bool          PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) const { return Connection->PostMessage(CmdId, RequestId, Message); }
	X_INLINE auto          operator->() const { return (xTcpServiceClientConnectionUserContext *)Connection; }

public:
	X_INLINE xTcpServiceClientConnectionHandle() = default;

private:
	friend class xTcpService;
	X_INLINE xTcpServiceClientConnectionHandle(xTcpService * Owner, xTcpServiceClientConnection * Connection)
		: Owner(Owner), Connection(Connection), ConnectionId(Connection -> ConnectionId) {
		Pass();
	}
	X_API_MEMBER xTcpServiceClientConnectionHandle(xTcpService * Owner, uint64_t ConnectionId);

	xTcpService *                 Owner        = nullptr;  //
	xTcpServiceClientConnection * Connection   = nullptr;  // valid in callbacks,
	uint64_t                      ConnectionId = 0;        // always checked by owner, use this for safety
};

class xTcpService final
	: private xTcpServer::iListener
	, private xTcpConnection::iListener
	, private xAbstract {
public:
	X_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress, size_t MaxConnectionId, bool ReuseAddress = true);
	X_API_MEMBER void Tick(uint64_t UpdatedNowMS);
	X_API_MEMBER void Clean();

public:
	using xOnClientConnected = std::function<void(const xTcpServiceClientConnectionHandle &)>;
	using xOnClientKeepAlive = std::function<void(const xTcpServiceClientConnectionHandle &)>;
	using xOnClientClose     = std::function<void(const xTcpServiceClientConnectionHandle &)>;
	using xOnClientClean     = std::function<void(const xTcpServiceClientConnectionHandle &)>;
	using xOnClientPacket    = std::function<bool(const xTcpServiceClientConnectionHandle &, xPacketCommandId, xPacketRequestId, ubyte *, size_t)>;

	xOnClientConnected OnClientConnected = Noop<>;
	xOnClientKeepAlive OnClientKeepAlive = Noop<>;
	xOnClientClose     OnClientClose     = Noop<>;
	xOnClientClean     OnClientClean     = Noop<>;
	xOnClientPacket    OnClientPacket    = Noop<true>;

public:
	X_API_MEMBER void SetMaxWriteBuffer(size_t Size);

	X_INLINE void DeferKillConnection(xTcpServiceClientConnection & Connection) {
		Connection.SetFlag_Killed();
		TcpServiceConnectionKillList.GrabTail(Connection);
	}

	X_INLINE void KeepAlive(xTcpServiceClientConnection & Connection) {
		assert(!Connection.HasFlag_Killed());
		Connection.TimestampMS = Ticker();
		TcpServiceConnectionTimeoutList.GrabTail(Connection);
	}

	X_INLINE xTcpServiceClientConnectionHandle GetConnectionHandle(uint64_t ConnectionId) { return { this, ConnectionId }; }

private:
	friend class xTcpServiceClientConnection;
	friend class xTcpServiceClientConnectionHandle;

	X_API_MEMBER xTcpServiceClientConnection * GetConnection(uint64_t ConnectionId) const;

private:
	X_PRIVATE_MEMBER void   OnNewConnection(xTcpServer * TcpServerPtr, xSocket && NativeHandle) override;
	X_PRIVATE_MEMBER void   OnPeerClose(xTcpConnection * TcpConnectionPtr) override;
	X_PRIVATE_MEMBER size_t OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) override;

	X_PRIVATE_MEMBER void CleanupConnection(xTcpServiceClientConnection & Connection);
	X_PRIVATE_MEMBER void CleanupKilledConnections();

	[[nodiscard]] X_STATIC_INLINE xTcpServiceClientConnection & Cast(xTcpConnection & Connection) { return static_cast<xTcpServiceClientConnection &>(Connection); };
	[[nodiscard]] X_STATIC_INLINE xTcpServiceClientConnection & Cast(xTcpServiceClientConnectionNode & Node) { return static_cast<xTcpServiceClientConnection &>(Node); };

private:
	// config
	size_t MaxWriteBufferLimitForEachConnection = 100'000'000 / sizeof(xPacketBuffer::Buffer);

	//
	xTicker                                        Ticker;
	xTcpServer                                     TcpServer;
	xIndexedStorage<xTcpServiceClientConnection *> ConnectionIdPool;

	xTcpServiceClientConnectionList TcpServiceConnectionTimeoutList;
	xTcpServiceClientConnectionList TcpServiceConnectionKillList;
};

X_END
