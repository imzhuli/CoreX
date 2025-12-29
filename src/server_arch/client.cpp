#include "./client.hpp"

#include "../core/string.hpp"

#include <cinttypes>

X_BEGIN

static constexpr const uint64_t MaxKeepAliveTimeoutMS     = 90'000;
static constexpr const uint64_t IdleTimeoutMS             = 30'000 + MaxKeepAliveTimeoutMS;
static constexpr const int64_t  ReconnectTimeoutMS        = 3'000;
static constexpr const int64_t  RequestKeepAliveTimeoutMS = 10'000;

bool xClient::Init(xIoContext * IoContextPtr, const xNetAddress & TargetAddress, const xNetAddress & BindAddress) {
	assert(IoContextPtr);
	assert(TargetAddress);

	this->IoContextPtr  = IoContextPtr;
	this->TargetAddress = TargetAddress;
	this->BindAddress   = BindAddress;
	SetDefaultKeepAliveTimeout();
	return true;
}

void xClient::Clean() {
	if (IsOpen()) {
		Connection.Clean();
	}

	Reset(NowMS);
	Reset(ReconnectTimestampMS);
	Reset(KeepAliveTimeoutMS);
	Reset(LastKeepAliveTimestampMS);
	Reset(LastRequestKeepAliveTimestampMS);
	Reset(KillConnection);

	X_DEBUG_RESET(IoContextPtr);
	X_DEBUG_RESET(TargetAddress);
	X_DEBUG_RESET(ReconnectTimestampMS);
}

void xClient::OnConnected(xTcpConnection * TcpConnectionPtr) {
	OnServerConnected();
}

void xClient::OnPeerClose(xTcpConnection * TcpConnectionPtr) {
	OnServerDisconnected();
	KillConnection = true;
}

void xClient::Kill() {
	if (!IsOpen()) {
		return;
	}
	KillConnection = true;
}

void xClient::Tick(uint64_t NowMS) {
	if (Steal(KillConnection)) {
		assert(IsOpen());
		Connection.Clean();
		LastKeepAliveTimestampMS        = 0;
		LastRequestKeepAliveTimestampMS = 0;
		return;
	}
	if (IsOpen()) {
		auto IdleTime = SignedDiff(NowMS, LastKeepAliveTimestampMS);
		if (IdleTime > MakeSigned(IdleTimeoutMS)) {
			KillConnection = true;
			return;
		}
		// force keepalive:
		if (KeepAliveTimeoutMS && IdleTime > MakeSigned(KeepAliveTimeoutMS)) {
			assert(KeepAliveTimeoutMS < IdleTimeoutMS);
			if (SignedDiff(NowMS, LastRequestKeepAliveTimestampMS) < RequestKeepAliveTimeoutMS) {
				// prevent continuously send request_keepalive
				return;
			}
			PostRequestKeepAlive();
			LastRequestKeepAliveTimestampMS = NowMS;
		}
		return;
	}

	// try reconnect
	if (SignedDiff(NowMS, ReconnectTimestampMS) < ReconnectTimeoutMS) {
		return;
	}
	ReconnectTimestampMS = NowMS;
	if (!Connection.Init(IoContextPtr, TargetAddress, BindAddress, this)) {
		return;
	} else {
		Connection.SetMaxWriteBufferSize(MaxWriteBufferLimitForEachConnection);
		LastKeepAliveTimestampMS        = NowMS;
		LastRequestKeepAliveTimestampMS = NowMS;
	}
}

size_t xClient::OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) {
	assert(TcpConnectionPtr == &Connection);
	size_t RemainSize = DataSize;
	while (RemainSize >= PacketHeaderSize) {
		auto Header = xPacketHeader::Parse(DataPtr);
		if (!Header) { /* header error */
			return InvalidDataSize;
		}
		auto PacketSize = Header.PacketSize;  // make a copy, so Header can be reused
		if (RemainSize < PacketSize) {        // wait for data
			break;
		}
		if (Header.IsKeepAlive()) {
			// X_DEBUG_PRINTF("KeepAlive");
			LastKeepAliveTimestampMS = NowMS;
		} else {
			auto PayloadPtr  = xPacket::GetPayloadPtr(DataPtr);
			auto PayloadSize = Header.GetPayloadSize();
			if (!OnServerPacket(Header.CommandId, Header.RequestId, PayloadPtr, PayloadSize)) { /* packet error */
				return InvalidDataSize;
			}
		}
		DataPtr    += PacketSize;
		RemainSize -= PacketSize;
	}
	return DataSize - RemainSize;
}

void xClient::DisableKeepAliveOnTick() {
	SetKeepAliveTimeout(0);
}

void xClient::SetDefaultKeepAliveTimeout() {
	SetKeepAliveTimeout(MaxKeepAliveTimeoutMS);
}

void xClient::SetKeepAliveTimeout(uint64_t TimeoutMS) {
	KeepAliveTimeoutMS = std::min(TimeoutMS, MaxKeepAliveTimeoutMS);
}

void xClient::SetMaxWriteBuffer(size_t Size) {
	MaxWriteBufferLimitForEachConnection = (Size / sizeof(xPacketBuffer::Buffer)) + 1;
}

void xClient::PostRequestKeepAlive() {
	if (!Connection.IsConnected() || KillConnection) {
		return;
	}
	Connection.PostRequestKeepAlive();
}

void xClient::PostData(const void * DataPtr, size_t DataSize) {
	if (!Connection.IsConnected() || KillConnection) {
		return;
	}
	Connection.PostData(DataPtr, DataSize);
}

void xClient::PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) {
	ubyte Buffer[MaxPacketSize];
	auto  PSize = WriteMessage(Buffer, CmdId, RequestId, Message);
	if (!PSize) {
		return;
	}
	PostData(Buffer, PSize);
}

X_END
