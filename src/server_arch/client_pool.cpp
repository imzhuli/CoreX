#include "./client_pool.hpp"

#include "../core/string.hpp"

X_BEGIN

static constexpr const uint64_t MaxRequestKeepAliveTimeoutMS = 90'000;
static constexpr const uint64_t MaxConnectTimeoutMS          = 5'000;
static constexpr const uint64_t MaxKeepAliveTimeoutMS        = 5'000;
static constexpr const uint64_t AutoReconnectTimeoutMS       = 3'000;

// client connection:

X_MEMBER bool xClientPoolConnection::PostData(const void * DataPtr, size_t DataSize) {
	if (IsConnected()) {
		return false;
	}
	xTcpConnection::PostData(DataPtr, DataSize);
	return true;
}

bool xClientPoolConnection::PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) {
	if (!IsConnected()) {
		return false;
	}

	ubyte Buffer[MaxPacketSize];
	auto  PSize = WriteMessage(Buffer, CmdId, RequestId, Message);
	if (!PSize) {
		return false;
	}
	xTcpConnection::PostData(Buffer, PSize);
	return true;
}

// client connection handle

xClientPoolConnectionHandle::xClientPoolConnectionHandle(xClientPool * Owner, uint64_t ConnectionId)
	: Owner(Owner), Connection(Owner->GetConnection(ConnectionId)), ConnectionId(ConnectionId) {
	Pass();
}

bool xClientPoolConnectionHandle::IsValid() const {
	return Owner && (Connection == Owner->GetConnection(ConnectionId));
}

// client pool

bool xClientPool::Init(xIoContext * ICP, size_t MaxConnectionCount) {
	this->ICP = ICP;
	return ConnectionPool.Init(MaxConnectionCount);
}

void xClientPool::Clean() {
	KillConnectionList.GrabListTail(WaitForConnectionEstablishmentQueue);
	KillConnectionList.GrabListTail(RequestKeepAliveQueue);
	KillConnectionList.GrabListTail(WaitForKeepAliveQueue);
	KillAllConnections();

	ReleaseConnectionList.GrabListTail(AutoConnectionList);
	ReleaseConnections();
	ConnectionPool.Clean();
}

void xClientPool::Tick() {
	Tick(GetTimestampMS());
}

void xClientPool::Tick(uint64_t NowMS) {
	Ticker.Update(NowMS);
	CheckTimeoutConnections();
	KillAllConnections();
	ReleaseConnections();
	DoRequestKeepAlive();
	DoAutoReconnect();
}

uint64_t xClientPool::AddServer(const xNetAddress & Address) {
	if (!Address) {
		return 0;
	}
	auto CID = ConnectionPool.Acquire();
	if (!CID) {
		return 0;
	}
	auto & Conn = ConnectionPool[CID];

	Conn.Owner           = this;
	Conn.ConnectionId    = CID;
	Conn.InitTimestampMS = 0;
	Conn.TargetAddress   = Address;
	AutoConnectionList.AddTail(Conn);
	return CID;
}

xClientPoolConnection * xClientPool::GetConnection(uint64_t ConnectionId) {
	return ConnectionPool.CheckAndGet(ConnectionId);
}

void xClientPool::RemoveServer(uint64_t ConnectionId) {
	auto CCP = ConnectionPool.CheckAndGet(ConnectionId);
	if (!CCP) {
		return;
	}
	CCP->ReleaseMark = true;
	KillConnectionList.GrabTail(*CCP);
}

void xClientPool::DoRequestKeepAlive() {
	auto NowMS     = Ticker();
	auto Timepoint = NowMS - MaxRequestKeepAliveTimeoutMS;
	while (auto PC = static_cast<xClientPoolConnection *>(RequestKeepAliveQueue.PopHead([Timepoint](const xClientPoolConnectionTimeoutNode & N) {
			   return N.LastKeepAliveTimestampMS <= Timepoint;
		   }))) {
		X_DEBUG_PRINTF("ConnectionId=%" PRIx64 ", TargetAddress=%s", PC->ConnectionId, PC->TargetAddress.ToString().c_str());
		PC->PostRequestKeepAlive();
		PC->LastRequestKeepAliveTimestampMS = NowMS;
		WaitForKeepAliveQueue.AddTail(*PC);
	}
}

void xClientPool::DoAutoReconnect() {
	auto NowMS      = Ticker();
	auto Timepoint  = NowMS - AutoReconnectTimeoutMS;
	auto FailedList = xList<xClientPoolConnectionTimeoutNode>();
	while (auto PC = static_cast<xClientPoolConnection *>(AutoConnectionList.PopHead([Timepoint](const xClientPoolConnectionTimeoutNode & N) {
			   return N.InitTimestampMS <= Timepoint;
		   }))) {
		assert(!PC->IsOpen());
		X_DEBUG_PRINTF("Starting to init connection: %" PRIx64 ", TargetAddress=%s", PC->ConnectionId, PC->TargetAddress.ToString().c_str());
		PC->InitTimestampMS = NowMS;
		if (!PC->Init(ICP, PC->TargetAddress, this)) {
			FailedList.AddTail(*PC);
			continue;
		}
		WaitForConnectionEstablishmentQueue.AddTail(*PC);
	}
	AutoConnectionList.GrabListTail(FailedList);
}

void xClientPool::CheckTimeoutConnections() {
	auto NowMS              = Ticker();
	auto ConnectTimepoint   = NowMS - MaxConnectTimeoutMS;
	auto KeepAliveTimepoint = NowMS - MaxKeepAliveTimeoutMS;

	while (auto PC = static_cast<xClientPoolConnection *>(WaitForConnectionEstablishmentQueue.PopHead([Timepoint = ConnectTimepoint](const xClientPoolConnectionTimeoutNode & N
																									  ) { return N.InitTimestampMS <= Timepoint; }))) {
		X_DEBUG_PRINTF("Kill connect timeout connection, cid=%" PRIx64 "", PC->ConnectionId);
		KillConnectionList.GrabTail(*PC);
	}
	while (auto PC = static_cast<xClientPoolConnection *>(WaitForKeepAliveQueue.PopHead([Timepoint = KeepAliveTimepoint](const xClientPoolConnectionTimeoutNode & N) {
			   return N.LastRequestKeepAliveTimestampMS <= Timepoint;
		   }))) {
		X_DEBUG_PRINTF("Killing timeout connection, ConnectionId=%" PRIx64 "", PC->ConnectionId);
		KillConnectionList.GrabTail(*PC);
	}
}

void xClientPool::KillAllConnections() {
	while (auto PC = static_cast<xClientPoolConnection *>(KillConnectionList.PopHead())) {
		X_DEBUG_PRINTF(
			"Killing ConnectionId=%" PRIx64 ", TargetAddress=%s, Open=%s, Connected=%s, ReleaseMark=%s", PC->ConnectionId, PC->TargetAddress.ToString().c_str(),
			YN(PC->IsOpen()), YN(PC->IsConnected()), YN(PC->ReleaseMark)
		);
		if (PC->IsOpen()) {
			if (PC->IsConnected()) {  // OnConnected is called, OnTargetClose may or maynot be called
				OnTargetClean({ this, PC });
			}
			PC->Clean();
		}

		if (PC->ReleaseMark) {
			ReleaseConnectionList.AddTail(*PC);
		} else {
			PC->InitTimestampMS = Ticker();
			AutoConnectionList.AddTail(*PC);
		}
	}
}

void xClientPool::ReleaseConnections() {
	while (auto PC = static_cast<xClientPoolConnection *>(ReleaseConnectionList.PopHead())) {
		assert(PC == ConnectionPool.CheckAndGet(PC->ConnectionId));
		X_DEBUG_PRINTF("ConnectionId=%" PRIx64 ", TargetAddress=%s", PC->ConnectionId, PC->TargetAddress.ToString().c_str());
		ConnectionPool.Release(PC->ConnectionId);
	}
}

void xClientPool::DoKeepAlive(xClientPoolConnection * PC) {
	assert(PC->IsOpen());
	if (PC->ReleaseMark) {
		return;
	}
	X_DEBUG_PRINTF("ConnectionId=%" PRIx64 ", TargetAddress=%s", PC->ConnectionId, PC->TargetAddress.ToString().c_str());
	PC->LastKeepAliveTimestampMS = Ticker();
	RequestKeepAliveQueue.GrabTail(*PC);
}

void xClientPool::OnConnected(xTcpConnection * TcpConnectionPtr) {
	auto PC = static_cast<xClientPoolConnection *>(TcpConnectionPtr);
	X_DEBUG_PRINTF("ConnectionId=%" PRIx64 ", TargetAddress=%s", PC->ConnectionId, PC->TargetAddress.ToString().c_str());
	DoKeepAlive(PC);
	EstablishedConnectionList.AddTail(*PC);
	OnTargetConnected({ this, PC });
}

void xClientPool::OnPeerClose(xTcpConnection * TcpConnectionPtr) {
	auto PC = static_cast<xClientPoolConnection *>(TcpConnectionPtr);
	X_DEBUG_PRINTF("ConnectionId=%" PRIx64 ", TargetAddress=%s", PC->ConnectionId, PC->TargetAddress.ToString().c_str());
	if (PC->IsConnected()) {
		OnTargetClose({ this, PC });
	}
	EstablishedConnectionList.Remove(*PC);
	KillConnectionList.GrabTail(*PC);
}

size_t xClientPool::OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) {

	auto PC = static_cast<xClientPoolConnection *>(TcpConnectionPtr);
	assert(PC == static_cast<xClientPoolConnection *>(ConnectionPool.CheckAndGet(PC->ConnectionId)));
	// X_DEBUG_PRINTF("\n%s", HexShow(DataPtr, DataSize).c_str());

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
			DoKeepAlive(PC);
		} else {
			auto PayloadPtr  = xPacket::GetPayloadPtr(DataPtr);
			auto PayloadSize = Header.GetPayloadSize();
			if (!OnTargetPacket({ this, PC }, Header.CommandId, Header.RequestId, PayloadPtr, PayloadSize)) { /* packet error */
				return InvalidDataSize;
			}
		}
		DataPtr    += PacketSize;
		RemainSize -= PacketSize;
	}
	return DataSize - RemainSize;
}

bool xClientPool::PostData(const void * DataPtr, size_t DataSize) {
	auto PC = static_cast<xClientPoolConnection *>(EstablishedConnectionList.PopHead());
	if (!PC) {
		return false;
	}
	EstablishedConnectionList.AddTail(*PC);
	return PC->PostData(DataPtr, DataSize);
}

bool xClientPool::PostData(uint64_t ConnectionId, const void * DataPtr, size_t DataSize) {
	auto PC = GetConnection(ConnectionId);
	return PC && PC->PostData(DataPtr, DataSize);
}

bool xClientPool::PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) {
	auto PC = static_cast<xClientPoolConnection *>(EstablishedConnectionList.PopHead());
	if (!PC) {
		return false;
	}
	EstablishedConnectionList.AddTail(*PC);
	return PC->PostMessage(CmdId, RequestId, Message);
}

bool xClientPool::PostMessage(uint64_t ConnectionId, xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) {
	auto PC = GetConnection(ConnectionId);
	return PC && PC->PostMessage(CmdId, RequestId, Message);
}

X_END
