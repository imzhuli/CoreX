#pragma once
#include "../core/core_time.hpp"
#include "../core/indexed_storage.hpp"
#include "../network/tcp_connection.hpp"
#include "./message.hpp"

X_BEGIN

class xClientConnection;
class xClientPool;

struct xClientConnectionTimeoutNode : xListNode {
	union {
		uint64_t InitTimestampMS;
		uint64_t LastRequestKeepAliveTimestampMS;
		uint64_t LastKeepAliveTimestampMS;
	};
};

class xClientConnection
	: public xTcpConnection
	, public xClientConnectionTimeoutNode {
	friend class xClientPool;

public:
private:
	uint64_t    ConnectionId;
	xNetAddress TargetAddress;
	bool        ReleaseMark = false;
};

class xClientPool
	: public xTcpConnection::iListener
	, xAbstract {

public:
	bool Init(xIoContext * ICP, size_t MaxConnectionCount = 1024);
	void Clean();
	void Tick();
	void Tick(uint64_t NowMS);

	uint64_t            AddServer(const xNetAddress & Address);
	xClientConnection * GetConnection(uint64_t ConnectionId);
	void                RemoveServer(uint64_t ConnectionId);

	bool PostData(uint64_t ConnectionId, const void * DataPtr, size_t DataSize);
	bool PostData(xClientConnection * PC, const void * DataPtr, size_t DataSize);
	bool PostMessage(uint64_t ConnectionId, xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);
	bool PostMessage(xClientConnection * PC, xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);

protected:
	virtual void OnServerConnected(xClientConnection * PC);
	virtual void OnServerClose(xClientConnection * PC);
	virtual bool OnServerPacket(xClientConnection * PC, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize);

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

private:
	xIoContext *                       ICP = nullptr;
	xIndexedStorage<xClientConnection> ConnectionPool;
	xTicker                            Ticker;

	xList<xClientConnectionTimeoutNode> ReleaseConnectionList;
	xList<xClientConnectionTimeoutNode> KillConnectionList;
	xList<xClientConnectionTimeoutNode> AutoConnectionList;
	xList<xClientConnectionTimeoutNode> WaitForConnectionEstablishmentQueue;
	xList<xClientConnectionTimeoutNode> RequestKeepAliveQueue;
	xList<xClientConnectionTimeoutNode> WaitForKeepAliveQueue;
};

X_END
