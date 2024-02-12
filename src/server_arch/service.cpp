#include "./service.hpp"

#include "../core/core_chrono.hpp"
#include "../core/string.hpp"
#include "../network/packet.hpp"

#include <algorithm>
#include <cinttypes>

X_BEGIN

static constexpr const uint64_t KeepAliveTimeoutMS     = 105'000;
static constexpr const size_t   DefaultMinConnectionId = 1024;
static constexpr const size_t   DefaultMaxConnectionId = 50'000;

static ubyte  KeepAliveBuffer[128];
static size_t KeepAliveSize = xPacketHeader::MakeKeepAlive(KeepAliveBuffer);

bool xService::Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress, bool ReusePort) {
	return Init(IoContextPtr, BindAddress, DefaultMaxConnectionId, ReusePort);
}

bool xService::Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress, size_t MaxConnectionId, bool ReusePort) {
	NowMS = GetTimestampMS();
	if (!TcpServer.Init(IoContextPtr, BindAddress, this, ReusePort)) {
		return false;
	}
	auto TcpServerCleaner = MakeResourceCleaner(TcpServer);

	MaxConnectionId = std::max(DefaultMinConnectionId, MaxConnectionId);
	if (!ConnectionIdPool.Init(MaxConnectionId)) {
		return false;
	}
	auto ConnectionIdPoolCleaner = MakeResourceCleaner(ConnectionIdPool);

	TcpServerCleaner.Dismiss();
	ConnectionIdPoolCleaner.Dismiss();
	return true;
}

void xService::Clean() {
	ServiceConnectionKillList.GrabListTail(ServiceConnectionTimeoutList);
	CleanupKilledConnections();

	auto TcpServerCleaner = MakeResourceCleaner(TcpServer);
}

void xService::Tick() {
	NowMS              = GetTimestampMS();
	auto KillTimepoint = NowMS - KeepAliveTimeoutMS;
	for (auto & Node : ServiceConnectionTimeoutList) {
		if (SignedDiff(Node.TimestampMS, KillTimepoint) > 0) {
			break;
		}
		ServiceConnectionKillList.GrabTail(Node);
	}
	CleanupKilledConnections();
}

void xService::CleanupConnection(xServiceConnection & Connection) {
	X_DEBUG_PRINTF("ConnectionId=%" PRIu64 "", Connection.ConnectionId());
	assert(ConnectionIdPool.Check(Connection.ConnectionId));
	ConnectionIdPool.Release(Connection.ConnectionId);
	Connection.Clean();
	delete &Connection;
}

void xService::CleanupKilledConnections() {
	for (auto & Node : ServiceConnectionKillList) {
		auto & Connection = Cast(Node);
		CleanupConnection(Connection);
	}
}

void xService::OnNewConnection(xTcpServer * TcpServerPtr, xSocket && NativeHandle) {
	auto Connection = new (std::nothrow) xServiceConnection();
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
	auto IdReleaser = xScopeGuard([=] { ConnectionIdPool.Release(ConnectionId); });

	Deleter.Dismiss();
	Cleaner.Dismiss();
	IdReleaser.Dismiss();

	Connection->ConnectionId = ConnectionId;
	Connection->TimestampMS  = NowMS;
	ServiceConnectionTimeoutList.AddTail(*Connection);
	X_DEBUG_PRINTF("Success");
}

void xService::OnPeerClose(xTcpConnection * TcpConnectionPtr) {
	auto & Connection = Cast(*TcpConnectionPtr);
	X_DEBUG_PRINTF("OnPeerClose: %" PRIu64 "", Connection.ConnectionId);
	DeferKillConnection(Connection);
}

size_t xService::OnData(xTcpConnection * TcpConnectionPtr, void * DataPtrInput, size_t DataSize) {
	auto & Connection = Cast(*TcpConnectionPtr);
	auto   DataPtr    = static_cast<ubyte *>(DataPtrInput);
	size_t RemainSize = DataSize;
	while (RemainSize >= PacketHeaderSize) {
		auto Header = xPacketHeader::Parse(DataPtr);
		if (!Header) { /* header error */
			return InvalidPacketSize;
		}
		auto PacketSize = Header.PacketSize;  // make a copy, so Header can be reused
		if (RemainSize < PacketSize) {        // wait for data
			break;
		}
		if (Header.IsRequestKeepAlive()) {
			X_DEBUG_PRINTF("RequestKeepAlive: %" PRIu64 "", Connection.ConnectionId());
			if (!PostData(Connection, KeepAliveBuffer, KeepAliveSize)) {
				return InvalidPacketSize;
			}
		} else {
			auto PayloadPtr  = xPacket::GetPayload(DataPtr);
			auto PayloadSize = Header.GetPayloadSize();
			if (!OnPacket(Connection, Header, PayloadPtr, PayloadSize)) { /* packet error */
				return InvalidPacketSize;
			}
		}
		DataPtr    += PacketSize;
		RemainSize -= PacketSize;
	}
	KeepAlive(Connection);
	return DataSize - RemainSize;
}

bool xService::OnPacket(xServiceConnection & Connection, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) {
	X_DEBUG_PRINTF(
		"CommandId: %" PRIx32 ", RequestId:%" PRIx64 ": %s", Header.CommandId, Header.RequestId, HexShow(PayloadPtr, PayloadSize).c_str()
	);
	return true;
}
void xService::SetMaxWriteBuffer(size_t Size) {
	MaxWriteBufferLimitForEachConnection = (Size / sizeof(xPacketBuffer::Buffer)) + 1;
}

bool xService::PostData(uint64_t ConnectionId, const void * DataPtr, size_t DataSize) {
	auto HolderPtr = ConnectionIdPool.CheckAndGet(ConnectionId);
	if (!HolderPtr) {
		return false;
	}
	auto ConnectionPtr = *HolderPtr;
	return PostData(*ConnectionPtr, DataPtr, DataSize);
}

bool xService::PostData(xServiceConnection & Connection, const void * DataPtr, size_t DataSize) {
	auto Posted = Connection.PostData(DataPtr, DataSize);
	if (Posted != DataSize) {
		DeferKillConnection(Connection);
		return false;
	}
	return true;
}

X_END
