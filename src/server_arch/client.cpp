#include "./client.hpp"

#include "../core/string.hpp"

#include <cinttypes>

X_BEGIN

static constexpr const int64_t ReconnectTimeoutMS = 15'000;

bool xClient::Init(xIoContext * IoContextPtr, const xNetAddress & TargetAddress) {
	assert(IoContextPtr);
	assert(TargetAddress);

	this->IoContextPtr         = IoContextPtr;
	this->TargetAddress        = TargetAddress;
	this->ReconnectTimestampMS = 0;
	return true;
}

void xClient::Clean() {
	if (Connected) {
		Connection.Clean();
	}
	X_DEBUG_RESET(IoContextPtr);
	X_DEBUG_RESET(TargetAddress);
	X_DEBUG_RESET(ReconnectTimestampMS);
}

void xClient::Tick(uint64_t NowMS) {
	this->NowMS = NowMS;
	if (Steal(KillConnection)) {
		assert(Connected);
		Connection.Clean();
		Connected = false;
		return;
	}
	if (Connected) {
		return;
	}
	if (SignedDiff(NowMS, ReconnectTimestampMS) < ReconnectTimeoutMS) {
		return;
	}
	ReconnectTimestampMS = NowMS;
	if (!Connection.Init(IoContextPtr, TargetAddress, this)) {
		return;
	} else {
		Connection.SetMaxWriteBufferSize(MaxWriteBufferLimitForEachConnection);
		Connected = true;
	}
}

size_t xClient::OnData(xTcpConnection * TcpConnectionPtr, void * DataPtrInput, size_t DataSize) {
	assert(TcpConnectionPtr == &Connection);
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
		auto PayloadPtr  = xPacket::GetPayload(DataPtr);
		auto PayloadSize = Header.GetPayloadSize();
		if (!OnPacket(Header, PayloadPtr, PayloadSize)) { /* packet error */
			return InvalidPacketSize;
		}
		DataPtr    += PacketSize;
		RemainSize -= PacketSize;
	}
	return DataSize - RemainSize;
}

bool xClient::OnPacket(const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) {
	X_DEBUG_PRINTF(
		"CommandId: %" PRIu32 ", RequestId:%" PRIu64 ": %s", Header.CommandId, Header.RequestId, HexShow(PayloadPtr, PayloadSize).c_str()
	);
	return true;
}

void xClient::PostData(const void * DataPtr, size_t DataSize) {
	if (!Connected || KillConnection) {
		return;
	}
	auto Posted = Connection.PostData(DataPtr, DataSize);
	if (Posted != DataSize) {
		KillConnection = true;
		return;
	}
	return;
}

X_END
