#pragma once
#include "../core/indexed_storage.hpp"
#include "../core/list.hpp"
#include "../network/tcp_connection.hpp"
#include "../network/tcp_server.hpp"
#include "../network/udp_channel.hpp"
#include "./base.hpp"

X_BEGIN

class xServiceClientConnectionNode : public xListNode {
public:
	uint64_t TimestampMS = 0;
};

class xServiceClientConnection
	: public xTcpConnection
	, public xServiceClientConnectionNode {
public:
	xIndexId ConnectionId = {};
};
using xServiceClientConnectionList = xList<xServiceClientConnectionNode>;

class xService
	: xTcpServer::iListener
	, xTcpConnection::iListener
	, xAbstract {
public:
	X_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress, bool ReusePort = false);
	X_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress, size_t MaxConnectionId, bool ReusePort = false);
	X_API_MEMBER void Tick();
	X_API_MEMBER void Tick(uint64_t UpdatedNowMS);
	X_API_MEMBER void Clean();

	X_INLINE uint64_t GetTickTimeMS() const {
		return NowMS;
	}

public:
	X_API_MEMBER void SetMaxWriteBuffer(size_t Size);
	X_API_MEMBER bool PostData(uint64_t ConnectionId, const void * DataPtr, size_t DataSize);
	X_API_MEMBER bool PostData(xServiceClientConnection & Connection, const void * DataPtr, size_t DataSize);

protected:
	X_PRIVATE_MEMBER virtual void OnClientClose(xServiceClientConnection & Connection);
	X_PRIVATE_MEMBER virtual bool OnPacket(xServiceClientConnection & Connection, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize);

	X_INLINE void KeepAlive(xServiceClientConnection & Connection) {
		Connection.TimestampMS = NowMS;
		ServiceConnectionTimeoutList.GrabTail(Connection);
	}
	X_INLINE void DeferKillConnection(xServiceClientConnection & Connection) {
		ServiceConnectionKillList.GrabTail(Connection);
	}

private:
	X_PRIVATE_MEMBER void   OnNewConnection(xTcpServer * TcpServerPtr, xSocket && NativeHandle) override;
	X_PRIVATE_MEMBER void   OnPeerClose(xTcpConnection * TcpConnectionPtr) override;
	X_PRIVATE_MEMBER size_t OnData(xTcpConnection * TcpConnectionPtr, void * DataPtr, size_t DataSize) override;

	X_PRIVATE_MEMBER void CleanupConnection(xServiceClientConnection & Connection);
	X_PRIVATE_MEMBER void CleanupKilledConnections();

	[[no_discard]] X_STATIC_INLINE xServiceClientConnection & Cast(xTcpConnection & Connection) {
		return static_cast<xServiceClientConnection &>(Connection);
	};
	[[no_discard]] X_STATIC_INLINE xServiceClientConnection & Cast(xServiceClientConnectionNode & Node) {
		return static_cast<xServiceClientConnection &>(Node);
	};

private:
	// config
	size_t MaxWriteBufferLimitForEachConnection = 50'000'000 / sizeof(xPacketBuffer::Buffer);

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
	X_API_MEMBER bool Init(xIoContext * IoContextPtr, int AddressFamily = AF_INET) {
		return xUdpChannel::Init(IoContextPtr, AddressFamily, this);
	}
	X_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress) {
		return xUdpChannel::Init(IoContextPtr, BindAddress, this);
	}
	X_API_MEMBER void Clean() {
		xUdpChannel::Clean();
	}
	using xUdpChannel::PostData;

protected:
	X_API_MEMBER
	virtual void OnPacket(const xNetAddress & RemoteAddress, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize);

private:
	X_PRIVATE_MEMBER void OnData(xUdpChannel * ChannelPtr, void * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress) override;
};

X_END
