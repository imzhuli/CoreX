#include "./client_pool.hpp"

#include "../core/string.hpp"

X_BEGIN

static constexpr const uint64_t MaxRequestKeepAliveTimeoutMS = 90'000;
static constexpr const uint64_t MaxConnectTimeoutMS          = 5'000;
static constexpr const uint64_t MaxKeepAliveTimeoutMS        = 5'000;
static constexpr const uint64_t AutoReconnectTimeoutMS       = 3'000;

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
	Ticker.Update();
	OnTick();
}

void xClientPool::Tick(uint64_t NowMS) {
	Ticker.Update(NowMS);
	OnTick();
}

void xClientPool::OnTick() {
	CheckTimeoutConnection();
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

	Conn.ConnectionId    = CID;
	Conn.InitTimestampMS = 0;
	Conn.TargetAddress   = Address;
	AutoConnectionList.AddTail(Conn);
	return CID;
}

xClientConnection * xClientPool::GetConnection(uint64_t ConnectionId) {
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
	while (auto PC = static_cast<xClientConnection *>(RequestKeepAliveQueue.PopHead([Timepoint](const xClientConnectionTimeoutNode & N) {
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
	auto FailedList = xList<xClientConnectionTimeoutNode>();
	while (auto PC =
			   static_cast<xClientConnection *>(AutoConnectionList.PopHead([Timepoint](const xClientConnectionTimeoutNode & N) { return N.InitTimestampMS <= Timepoint; }))) {
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

void xClientPool::CheckTimeoutConnection() {
	auto NowMS              = Ticker();
	auto ConnectTimepoint   = NowMS - MaxConnectTimeoutMS;
	auto KeepAliveTimepoint = NowMS - MaxKeepAliveTimeoutMS;

	while (auto PC = static_cast<xClientConnection *>(WaitForConnectionEstablishmentQueue.PopHead([Timepoint = ConnectTimepoint](const xClientConnectionTimeoutNode & N) {
			   return N.InitTimestampMS <= Timepoint;
		   }))) {
		X_DEBUG_PRINTF("Kill connect timeout connection, cid=%" PRIx64 "", PC->ConnectionId);
		KillConnectionList.GrabTail(*PC);
	}
	while (auto PC = static_cast<xClientConnection *>(WaitForKeepAliveQueue.PopHead([Timepoint = KeepAliveTimepoint](const xClientConnectionTimeoutNode & N) {
			   return N.LastRequestKeepAliveTimestampMS <= Timepoint;
		   }))) {
		X_DEBUG_PRINTF("Killing timeout connection, ConnectionId=%" PRIx64 "", PC->ConnectionId);
		KillConnectionList.GrabTail(*PC);
	}
}

void xClientPool::KillAllConnections() {
	while (auto PC = static_cast<xClientConnection *>(KillConnectionList.PopHead())) {
		X_DEBUG_PRINTF(
			"Killing ConnectionId=%" PRIx64 ", TargetAddress=%s, Open=%s, ReleaseMark=%s", PC->ConnectionId, PC->TargetAddress.ToString().c_str(), YN(PC->IsOpen()),
			YN(PC->ReleaseMark)
		);
		if (PC->IsOpen()) {
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
	while (auto PC = static_cast<xClientConnection *>(ReleaseConnectionList.PopHead())) {
		assert(PC == ConnectionPool.CheckAndGet(PC->ConnectionId));
		X_DEBUG_PRINTF("ConnectionId=%" PRIx64 ", TargetAddress=%s", PC->ConnectionId, PC->TargetAddress.ToString().c_str());
		ConnectionPool.Release(PC->ConnectionId);
	}
}

void xClientPool::OnKeepAlive(xClientConnection * PC) {
	assert(PC->IsOpen());
	if (PC->ReleaseMark) {
		return;
	}
	X_DEBUG_PRINTF("ConnectionId=%" PRIx64 ", TargetAddress=%s", PC->ConnectionId, PC->TargetAddress.ToString().c_str());
	PC->LastKeepAliveTimestampMS = Ticker();
	RequestKeepAliveQueue.GrabTail(*PC);
}

void xClientPool::OnConnected(xTcpConnection * TcpConnectionPtr) {
	auto PC = static_cast<xClientConnection *>(TcpConnectionPtr);
	X_DEBUG_PRINTF("ConnectionId=%" PRIx64 ", TargetAddress=%s", PC->ConnectionId, PC->TargetAddress.ToString().c_str());
	OnKeepAlive(PC);
	OnServerConnected(PC);
}

void xClientPool::OnPeerClose(xTcpConnection * TcpConnectionPtr) {
	auto PC = static_cast<xClientConnection *>(TcpConnectionPtr);
	X_DEBUG_PRINTF("ConnectionId=%" PRIx64 ", TargetAddress=%s", PC->ConnectionId, PC->TargetAddress.ToString().c_str());

	OnServerClose(PC);
	KillConnectionList.GrabTail(*PC);
}

size_t xClientPool::OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) {

	auto PC = static_cast<xClientConnection *>(TcpConnectionPtr);
	assert(PC == static_cast<xClientConnection *>(ConnectionPool.CheckAndGet(PC->ConnectionId)));
	X_DEBUG_PRINTF("\n%s", HexShow(DataPtr, DataSize).c_str());

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
			OnKeepAlive(PC);
		} else {
			auto PayloadPtr  = xPacket::GetPayloadPtr(DataPtr);
			auto PayloadSize = Header.GetPayloadSize();
			if (!OnServerPacket(PC, Header, PayloadPtr, PayloadSize)) { /* packet error */
				return InvalidDataSize;
			}
		}
		DataPtr    += PacketSize;
		RemainSize -= PacketSize;
	}
	return DataSize - RemainSize;
}

void xClientPool::OnServerConnected(xClientConnection * PC) {
	X_DEBUG_PRINTF("ConnectionId=%" PRIx64 ", TargetAddress=%s", PC->ConnectionId, PC->TargetAddress.ToString().c_str());
}

void xClientPool::OnServerClose(xClientConnection * PC) {
	X_DEBUG_PRINTF("ConnectionId=%" PRIx64 ", TargetAddress=%s", PC->ConnectionId, PC->TargetAddress.ToString().c_str());
}

bool xClientPool::OnServerPacket(xClientConnection * PC, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) {
	X_DEBUG_PRINTF("ConnectionId=%" PRIx64 ", TargetAddress=%s, CommandId=%" PRIx32 "", PC->ConnectionId, PC->TargetAddress.ToString().c_str(), Header.CommandId);
	return true;
}

bool xClientPool::PostData(uint64_t ConnectionId, const void * DataPtr, size_t DataSize) {
	auto PC = GetConnection(ConnectionId);
	if (!PC) {
		return false;
	}
	return PostData(PC, DataPtr, DataSize);
}

bool xClientPool::PostData(xClientConnection * PC, const void * DataPtr, size_t DataSize) {
	if (!PC->IsOpen()) {
		return false;
	}
	PC->PostData(DataPtr, DataSize);
	return true;
}

bool xClientPool::PostMessage(uint64_t ConnectionId, xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) {
	auto PC = GetConnection(ConnectionId);
	if (!PC) {
		return false;
	}
	return PostMessage(PC, CmdId, RequestId, Message);
}

bool xClientPool::PostMessage(xClientConnection * PC, xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) {
	if (!PC->IsOpen()) {
		return false;
	}

	ubyte Buffer[MaxPacketSize];
	auto  PSize = WritePacket(CmdId, RequestId, Buffer, Message);
	if (!PSize) {
		return false;
	}
	PC->PostData(Buffer, PSize);
	return true;
}

X_END
