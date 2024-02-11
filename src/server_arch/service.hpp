#pragma once
#include "../core/indexed_storage.hpp"
#include "../core/list.hpp"
#include "../network/tcp_connection.hpp"
#include "../network/tcp_server.hpp"
#include "./base.hpp"

X_BEGIN

class xServiceConnectionNode : public xListNode {
public:
	uint64_t TimestampMS = 0;
};

class xServiceConnection
	: public xTcpConnection
	, public xServiceConnectionNode {
public:
	xIndexId ConnectionId = {};
};
using xServiceConnectionList = xList<xServiceConnectionNode>;

class xService
	: xTcpServer::iListener
	, xTcpConnection::iListener
	, xAbstract {
public:
	X_PRIVATE_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress, bool ReusePort = false);
	X_PRIVATE_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress, size_t MaxConnectionId, bool ReusePort = false);
	X_PRIVATE_MEMBER void Tick();
	X_PRIVATE_MEMBER void Clean();

public:
	void PostData(uint64_t ConnectionId, const void * DataPtr, size_t DataSize);
	void PostData(xServiceConnection & Connection, const void * DataPtr, size_t DataSize);

protected:
	X_PRIVATE_MEMBER virtual bool OnPacket(
		xServiceConnection & Connection, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize
	);

	X_INLINE void KeepAlive(xServiceConnection & Connection) {
		Connection.TimestampMS = NowMS;
		ServiceConnectionTimeoutList.GrabTail(Connection);
	}
	X_INLINE void DeferKillConnection(xServiceConnection & Connection) {
		ServiceConnectionKillList.GrabTail(Connection);
	}

private:
	X_PRIVATE_MEMBER void   OnNewConnection(xTcpServer * TcpServerPtr, xSocket && NativeHandle) override;
	X_PRIVATE_MEMBER void   OnPeerClose(xTcpConnection * TcpConnectionPtr) override;
	X_PRIVATE_MEMBER size_t OnData(xTcpConnection * TcpConnectionPtr, void * DataPtr, size_t DataSize) override;

	X_PRIVATE_MEMBER void CleanupConnection(xServiceConnection & Connection);
	X_PRIVATE_MEMBER void CleanupKilledConnections();

	[[no_discard]] X_STATIC_INLINE xServiceConnection & Cast(xTcpConnection & Connection) {
		return static_cast<xServiceConnection &>(Connection);
	};
	[[no_discard]] X_STATIC_INLINE xServiceConnection & Cast(xServiceConnectionNode & Node) {
		return static_cast<xServiceConnection &>(Node);
	};

private:
	uint64_t                              NowMS;
	xTcpServer                            TcpServer;
	xIndexedStorage<xServiceConnection *> ConnectionIdPool;

	xServiceConnectionList ServiceConnectionTimeoutList;
	xServiceConnectionList ServiceConnectionKillList;
};

X_END
