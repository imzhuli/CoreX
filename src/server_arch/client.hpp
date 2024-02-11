#pragma once
#include "../network/tcp_connection.hpp"
#include "./base.hpp"

X_BEGIN

class xClient
	: public xTcpConnection::iListener
	, xAbstract {
public:
	X_PRIVATE_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & TargetAddress);
	X_PRIVATE_MEMBER void Tick(uint64_t NowMS);
	X_PRIVATE_MEMBER void Clean();

	X_INLINE bool IsConnected() const {
		return Connected;
	}
	X_PRIVATE_MEMBER void PostData(const void * DataPtr, size_t DataSize);

protected:
	X_PRIVATE_MEMBER bool OnPacket(const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize);

private:
	X_API_MEMBER void OnConnected(xTcpConnection * TcpConnectionPtr) override {
		X_DEBUG_PRINTF("");
	}
	X_API_MEMBER size_t OnData(xTcpConnection * TcpConnectionPtr, void * DataPtr, size_t DataSize) override;
	X_API_MEMBER void   OnPeerClose(xTcpConnection * TcpConnectionPtr) override {
        KillConnection = true;
	}

private:
	xIoContext *   IoContextPtr = {};
	xNetAddress    TargetAddress;
	xTcpConnection Connection;

	uint64_t NowMS                = 0;
	uint64_t ReconnectTimestampMS = 0;
	bool     Connected            = false;
	bool     KillConnection       = false;
};

X_END
