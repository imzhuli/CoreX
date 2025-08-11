#include "./tcp_service.hpp"

X_BEGIN

static constexpr const uint64_t KeepAliveTimeoutMS     = 105'000;
static constexpr const size_t   DefaultMinConnectionId = 1024;

//////////////////////////
// xTcpServiceClientConnection
//////////////////////////

void xTcpServiceClientConnection::PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) {
	ubyte Buffer[MaxPacketSize];
	auto  PSize = WriteMessage(Buffer, CmdId, RequestId, Message);
	if (!PSize) {
		return;
	}
	PostData(Buffer, PSize);
}

//////////////////////////
// xTcpServiceClientConnectionHandle
//////////////////////////

xTcpServiceClientConnection * xTcpServiceClientConnectionHandle::operator->() const {
	auto VC = Owner->GetConnection(ConnectionId);
	if (!VC) {
		return nullptr;
	}
	assert(VC == Connection);
	return VC;
}

xTcpServiceClientConnection & xTcpServiceClientConnectionHandle::operator*() const {
	return *Connection;
}

//////////////////////////
// xTcpService
//////////////////////////

bool xTcpService::Init(xIoContext * IoContextPtr, const xNetAddress & BindAddress, size_t MaxConnectionId, bool ReuseAddress) {
	Ticker.Update();
	if (!TcpServer.Init(IoContextPtr, BindAddress, this, ReuseAddress)) {
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

void xTcpService::Clean() {
	TcpServiceConnectionKillList.GrabListTail(TcpServiceConnectionTimeoutList);
	CleanupKilledConnections();

	auto TcpServerCleaner        = xScopeCleaner(TcpServer);
	auto ConnectionIdPoolCleaner = xScopeCleaner(ConnectionIdPool);
}

void xTcpService::Tick(uint64_t UpdatedNowMS) {
	Ticker.Update(UpdatedNowMS);
	auto Cond = [KillTimepoint = UpdatedNowMS - KeepAliveTimeoutMS](const xTcpServiceClientConnectionNode & N) { return SignedDiff(N.TimestampMS, KillTimepoint) < 0; };
	while (auto NP = TcpServiceConnectionTimeoutList.PopHead(Cond)) {
		TcpServiceConnectionKillList.GrabTail(*NP);
	}
	CleanupKilledConnections();
}

void xTcpService::CleanupKilledConnections() {
	while (auto NP = TcpServiceConnectionKillList.PopHead()) {
		CleanupConnection(Cast(*NP));
	}
}

void xTcpService::CleanupConnection(xTcpServiceClientConnection & Connection) {
	OnCleanupClientConnection({ this, &Connection });
	ConnectionIdPool.Release(Connection.ConnectionId);
	Connection.Clean();
	delete &Connection;
}

xTcpServiceClientConnection * xTcpService::GetConnection(uint64_t ConnectionId) const {
	auto HolderPtr = ConnectionIdPool.CheckAndGet(ConnectionId);
	return HolderPtr ? *HolderPtr : nullptr;
}

void xTcpService::SetMaxWriteBuffer(size_t Size) {
	MaxWriteBufferLimitForEachConnection = (Size / sizeof(xPacketBuffer::Buffer)) + 1;
}

void xTcpService::OnNewConnection(xTcpServer * TcpServerPtr, xSocket && NativeHandle) {
	X_DEBUG_PRINTF("");
	auto Connection = new (std::nothrow) xTcpServiceClientConnection();
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
	Connection->TimestampMS  = Ticker();
	TcpServiceConnectionTimeoutList.AddTail(*Connection);

	OnClientConnected({ this, Connection });
}

void xTcpService::OnPeerClose(xTcpConnection * TcpConnectionPtr) {
	auto & Connection = Cast(*TcpConnectionPtr);
	OnClientClose({ this, &Connection });
	DeferKillConnection(Connection);
}

size_t xTcpService::OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) {
	auto & Connection       = Cast(*TcpConnectionPtr);
	auto   ConnectionHandle = xTcpServiceClientConnectionHandle{ this, &Connection };

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
		if (Header.IsRequestKeepAlive() && !Connection.HasFlag_Killed()) {
			Connection.PostKeepAlive();
			KeepAlive(Connection);
			OnClientKeepAlive(ConnectionHandle);
		} else {
			auto PayloadPtr  = xPacket::GetPayloadPtr(DataPtr);
			auto PayloadSize = Header.GetPayloadSize();
			if (!OnClientPacket(ConnectionHandle, Header.CommandId, Header.RequestId, PayloadPtr, PayloadSize)) { /* packet error */
				return InvalidDataSize;
			}
		}
		DataPtr    += PacketSize;
		RemainSize -= PacketSize;
	}
	return DataSize - RemainSize;
}

X_END
