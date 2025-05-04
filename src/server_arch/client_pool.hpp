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
	inline xIndexId            GetConnectionId() const { return ConnectionId; }
	inline const xNetAddress & GetTargetAddress() const { return TargetAddress; }

public:
	xVariable UserContext = {};

private:
	xIndexId    ConnectionId;
	xNetAddress TargetAddress;
	bool        ReleaseMark = false;
};

class xClientPool
	: xTcpConnection::iListener
	, xAbstract {

public:
	bool Init(xIoContext * ICP, size_t MaxConnectionCount = 1024);
	void Clean();
	void Tick();
	void Tick(uint64_t NowMS);

	xIndexId AddServer(const xNetAddress & Address);
	void     RemoveServer(xIndexId ConnectionId);

	bool PostData(const void * DataPtr, size_t DataSize);
	bool PostData(uint64_t ConnectionId, const void * DataPtr, size_t DataSize);
	bool PostData(xClientConnection & PC, const void * DataPtr, size_t DataSize);
	bool PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);
	bool PostMessage(uint64_t ConnectionId, xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);
	bool PostMessage(xClientConnection & PC, xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);

protected:
	virtual void OnServerConnected(xClientConnection & CC);
	virtual void OnServerClose(xClientConnection & CC);
	virtual bool OnServerPacket(xClientConnection & CC, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);

private:
	void CheckTimeoutConnection();
	void KillAllConnections();
	void ReleaseConnections();
	void DoRequestKeepAlive();
	void DoAutoReconnect();

	void OnTick();
	void OnKeepAlive(xClientConnection * PC);

	void   OnConnected(xTcpConnection * TcpConnectionPtr) override;
	void   OnPeerClose(xTcpConnection * TcpConnectionPtr) override;
	size_t OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) override;

	xClientConnection * GetConnection(uint64_t ConnectionId);

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
