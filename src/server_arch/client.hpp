#pragma once
#include "../network/tcp_connection.hpp"

X_BEGIN

class xClient
	: public xTcpConnection::iListener
	, xAbstract {
public:
	X_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & TargetAddress) {
		return Init(IoContextPtr, TargetAddress, TargetAddress.Decay());
	}
	X_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & TargetAddress, const xNetAddress & BindAddress);
	X_API_MEMBER void Tick(uint64_t NowMS);
	X_API_MEMBER void Clean();

	X_INLINE bool IsConnected() const {
		return Connected;
	}
	X_API_MEMBER void DisableKeepAliveOnTick();
	X_API_MEMBER void SetDefaultKeepAliveTimeout();
	X_API_MEMBER void SetKeepAliveTimeout(uint64_t TimeoutMS);
	X_API_MEMBER void SetMaxWriteBuffer(size_t Size);
	X_API_MEMBER void PostData(const void * DataPtr, size_t DataSize);

protected:
	X_PRIVATE_MEMBER virtual void OnServerConnected();
	X_PRIVATE_MEMBER virtual void OnServerClose();
	X_PRIVATE_MEMBER virtual bool OnPacket(const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize);

private:
	X_API_MEMBER void   OnConnected(xTcpConnection * TcpConnectionPtr) override;
	X_API_MEMBER void   OnPeerClose(xTcpConnection * TcpConnectionPtr) override;
	X_API_MEMBER size_t OnData(xTcpConnection * TcpConnectionPtr, void * DataPtr, size_t DataSize) override;

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
	bool     Connected                       = false;
	bool     KillConnection                  = false;
};

X_END
