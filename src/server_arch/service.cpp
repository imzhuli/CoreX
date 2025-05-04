#include "./service.hpp"

#include "../core/core_time.hpp"
#include "../core/string.hpp"
#include "../network/packet.hpp"

#include <algorithm>
#include <cinttypes>

X_BEGIN

static constexpr const uint64_t KeepAliveTimeoutMS     = 105'000;
static constexpr const size_t   DefaultMinConnectionId = 1024;
static constexpr const size_t   DefaultMaxConnectionId = 50'000;

bool xService::Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress, bool ReusePort) {
	return Init(IoContextPtr, BindAddress, DefaultMaxConnectionId, ReusePort);
}

bool xService::Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress, size_t MaxConnectionId, bool ReusePort) {
	NowMS = GetTimestampMS();
	if (!TcpServer.Init(IoContextPtr, BindAddress, this)) {
		return false;
	}
	auto TcpServerCleaner = xScopeCleaner(TcpServer);

	MaxConnectionId = std::max(DefaultMinConnectionId, MaxConnectionId);
	if (!ConnectionIdPool.Init(MaxConnectionId)) {
		return false;
	}
	auto ConnectionIdPoolCleaner = xScopeCleaner(ConnectionIdPool);

	TcpServerCleaner.Dismiss();
	ConnectionIdPoolCleaner.Dismiss();
	return true;
}

void xService::Clean() {
	ServiceConnectionKillList.GrabListTail(ServiceConnectionTimeoutList);
	CleanupKilledConnections();

	auto TcpServerCleaner = xScopeCleaner(TcpServer);
}

void xService::Tick() {
	auto UpdatedNowMS = GetTimestampMS();
	Tick(UpdatedNowMS);
}

void xService::Tick(uint64_t UpdatedNowMS) {
	NowMS = UpdatedNowMS;

	auto Cond = [KillTimepoint = NowMS - KeepAliveTimeoutMS](const xServiceClientConnectionNode & N) { return SignedDiff(N.TimestampMS, KillTimepoint) < 0; };
	while (auto NP = ServiceConnectionTimeoutList.PopHead(Cond)) {
		ServiceConnectionKillList.GrabTail(*NP);
	}
	CleanupKilledConnections();
}

void xService::CleanupConnection(xServiceClientConnection & Connection) {
	assert(ConnectionIdPool.Check(Connection.ConnectionId));
	OnCleanupClientConnection(Connection);
	ConnectionIdPool.Release(Connection.ConnectionId);
	Connection.Clean();
	delete &Connection;
}

void xService::CleanupKilledConnections() {
	while (auto NP = ServiceConnectionKillList.PopHead()) {
		CleanupConnection(Cast(*NP));
	}
}

void xService::OnNewConnection(xTcpServer * TcpServerPtr, xSocket && NativeHandle) {
	X_DEBUG_PRINTF("");
	auto Connection = new (std::nothrow) xServiceClientConnection();
	if (!Connection) {
		XelCloseSocket(NativeHandle);
		return;
	}
	auto Deleter = xScopeGuard([=] { delete Connection; });

	if (!Connection->Init(TcpServer.GetIoContextPtr(), std::move(NativeHandle), this)) {
		return;
	}
	auto Cleaner = xScopeGuard([=] { Connection->Clean(); });
	Connection->SetMaxWriteBufferSize(MaxWriteBufferLimitForEachConnection);

	auto ConnectionId = ConnectionIdPool.Acquire(Connection);
	if (!ConnectionId) {
		return;
	}
	auto IdReleaser = xScopeGuard([=, this] { ConnectionIdPool.Release(ConnectionId); });

	Deleter.Dismiss();
	Cleaner.Dismiss();
	IdReleaser.Dismiss();

	Connection->ConnectionId = ConnectionId;
	Connection->TimestampMS  = NowMS;
	ServiceConnectionTimeoutList.AddTail(*Connection);

	OnClientConnected(*Connection);
}

void xService::OnClientConnected(xServiceClientConnection & Connection) {
	// X_DEBUG_PRINTF("ConnectionId: %" PRIx64 "", Connection.ConnectionId);
}

void xService::OnClientClose(xServiceClientConnection & Connection) {
	// X_DEBUG_PRINTF("ConnectionId: %" PRIx64 "", Connection.ConnectionId);
}

void xService::OnPeerClose(xTcpConnection * TcpConnectionPtr) {
	auto & Connection = Cast(*TcpConnectionPtr);
	OnClientClose(Connection);
	DeferKillConnection(Connection);
}

size_t xService::OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) {
	auto & Connection = Cast(*TcpConnectionPtr);
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
		if (Header.IsRequestKeepAlive() && !Connection.IsBeingKilled()) {
			// X_DEBUG_PRINTF("RequestKeepAlive: %" PRIx64 "", Connection.ConnectionId());
			Connection.PostKeepAlive();
			KeepAlive(Connection);
		} else {
			auto PayloadPtr  = xPacket::GetPayloadPtr(DataPtr);
			auto PayloadSize = Header.GetPayloadSize();
			if (!OnClientPacket(Connection, Header.CommandId, Header.RequestId, PayloadPtr, PayloadSize)) { /* packet error */
				return InvalidDataSize;
			}
		}
		DataPtr    += PacketSize;
		RemainSize -= PacketSize;
	}
	return DataSize - RemainSize;
}

bool xService::OnClientPacket(xServiceClientConnection & Connection, xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
	X_DEBUG_PRINTF("CommandId: %" PRIx32 ", RequestId:%" PRIx64 ":  \n%s", CommandId, RequestId, HexShow(PayloadPtr, PayloadSize).c_str());
	return true;
}

void xService::OnCleanupClientConnection(const xServiceClientConnection & Connection) {
}

void xService::SetMaxWriteBuffer(size_t Size) {
	MaxWriteBufferLimitForEachConnection = (Size / sizeof(xPacketBuffer::Buffer)) + 1;
}

void xService::PostData(uint64_t ConnectionId, const void * DataPtr, size_t DataSize) {
	auto HolderPtr = ConnectionIdPool.CheckAndGet(ConnectionId);
	if (!HolderPtr) {
		return;
	}
	auto ConnectionPtr = *HolderPtr;
	PostData(*ConnectionPtr, DataPtr, DataSize);
}

void xService::PostData(xServiceClientConnection & Connection, const void * DataPtr, size_t DataSize) {
	if (Connection.IsBeingKilled()) {
		return;
	}
	Connection.PostData(DataPtr, DataSize);
}

void xService::PostMessage(uint64_t ConnectionId, xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) {
	auto HolderPtr = ConnectionIdPool.CheckAndGet(ConnectionId);
	if (!HolderPtr) {
		return;
	}
	auto ConnectionPtr = *HolderPtr;
	PostMessage(*ConnectionPtr, CmdId, RequestId, Message);
}

void xService::PostMessage(xServiceClientConnection & Connection, xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) {
	ubyte Buffer[MaxPacketSize];
	auto  PSize = WritePacket(CmdId, RequestId, Buffer, Message);
	if (!PSize) {
		return;
	}
	PostData(Connection, Buffer, PSize);
}

/* udp */
void xUdpService::OnData(xUdpChannel * ChannelPtr, ubyte * DataPtr, size_t DataSize, const xNetAddress & RemoteAddress) {
	assert(this == ChannelPtr);
	if (DataSize < PacketHeaderSize) {
		return;
	}

	auto Header = xPacketHeader::Parse(DataPtr);
	if (!Header) { /* header error */
		return;
	}
	auto PacketSize = Header.PacketSize;  // make a copy, so Header can be reused
	if (DataSize != PacketSize) {         // wait for data
		return;
	}

	if (Header.IsRequestKeepAlive()) {
		ChannelPtr->PostKeepAlive(RemoteAddress);
		return;
	}

	auto PayloadPtr  = xPacket::GetPayloadPtr(DataPtr);
	auto PayloadSize = Header.GetPayloadSize();
	OnPacket(RemoteAddress, Header, PayloadPtr, PayloadSize);
	return;
}

void xUdpService::OnPacket(const xNetAddress & RemoteAddress, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) {
	X_DEBUG_PRINTF("CommandId: %" PRIx32 ", RequestId:%" PRIx64 ":  \n%s", Header.CommandId, Header.RequestId, HexShow(PayloadPtr, PayloadSize).c_str());
}

X_END
