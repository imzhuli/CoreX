#pragma once
#include "../core/core_time.hpp"
#include "../core/indexed_storage.hpp"
#include "../network/tcp_connection.hpp"
#include "./message.hpp"

X_BEGIN

class xClientConnection;
class xClientPool;

struct xCLientConnectionAvailableNode : xListNode {};
struct xClientConnectionTimeoutNode : xListNode {
	union {
		uint64_t InitTimestampMS;
		uint64_t LastRequestKeepAliveTimestampMS;
		uint64_t LastKeepAliveTimestampMS;
	};
};

class xClientConnection
	: public xTcpConnection
	, public xCLientConnectionAvailableNode
	, public xClientConnectionTimeoutNode {
	friend class xClientPool;

public:
	X_INLINE xIndexId            GetConnectionId() const { return ConnectionId; }
	X_INLINE const xNetAddress & GetTargetAddress() const { return TargetAddress; }
	X_INLINE void                SetUserContext(xVariable V) { UserContext = V; }
	X_INLINE xVariable &         GetUserContext() { return UserContext; }
	X_INLINE const xVariable &   GetUserContext() const { return UserContext; }

private:
	xIndexId    ConnectionId;
	xNetAddress TargetAddress;
	xVariable   UserContext = {};
	bool        ReleaseMark = false;
};

class xClientPool
	: xTcpConnection::iListener
	, xAbstract {

public:
	X_API_MEMBER bool Init(xIoContext * ICP, size_t MaxConnectionCount = 1024);
	X_API_MEMBER void Clean();
	X_API_MEMBER void Tick();
	X_API_MEMBER void Tick(uint64_t NowMS);

	X_API_MEMBER xIndexId AddServer(const xNetAddress & Address);
	X_API_MEMBER void     RemoveServer(xIndexId ConnectionId);

	X_API_MEMBER bool PostData(const void * DataPtr, size_t DataSize);
	X_API_MEMBER bool PostData(uint64_t ConnectionId, const void * DataPtr, size_t DataSize);
	X_API_MEMBER bool PostData(xClientConnection & PC, const void * DataPtr, size_t DataSize);
	X_API_MEMBER bool PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);
	X_API_MEMBER bool PostMessage(uint64_t ConnectionId, xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);
	X_API_MEMBER bool PostMessage(xClientConnection & PC, xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);

	X_INLINE uint64_t GetTickTimeMS() const { return Ticker(); }

protected:
	X_API_MEMBER virtual void OnTick(uint64_t NowMS);
	X_API_MEMBER virtual void OnServerConnected(xClientConnection & CC);
	X_API_MEMBER virtual void OnServerClose(xClientConnection & CC);
	X_API_MEMBER virtual bool OnServerPacket(xClientConnection & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);

private:
	X_PRIVATE_MEMBER void CheckTimeoutConnections();
	X_PRIVATE_MEMBER void KillAllConnections();
	X_PRIVATE_MEMBER void ReleaseConnections();
	X_PRIVATE_MEMBER void DoRequestKeepAlive();
	X_PRIVATE_MEMBER void DoAutoReconnect();
	X_PRIVATE_MEMBER void DoKeepAlive(xClientConnection * PC);

	X_PRIVATE_MEMBER void   OnConnected(xTcpConnection * TcpConnectionPtr) override;
	X_PRIVATE_MEMBER void   OnPeerClose(xTcpConnection * TcpConnectionPtr) override;
	X_PRIVATE_MEMBER size_t OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) override;

	X_PRIVATE_MEMBER xClientConnection * GetConnection(uint64_t ConnectionId);

private:
	xIoContext *                       ICP = nullptr;
	xIndexedStorage<xClientConnection> ConnectionPool;
	xTicker                            Ticker;

	xList<xCLientConnectionAvailableNode> EstablishedConnectionList;
	xList<xClientConnectionTimeoutNode>   ReleaseConnectionList;
	xList<xClientConnectionTimeoutNode>   KillConnectionList;
	xList<xClientConnectionTimeoutNode>   AutoConnectionList;
	xList<xClientConnectionTimeoutNode>   WaitForConnectionEstablishmentQueue;
	xList<xClientConnectionTimeoutNode>   RequestKeepAliveQueue;
	xList<xClientConnectionTimeoutNode>   WaitForKeepAliveQueue;
};

X_END
