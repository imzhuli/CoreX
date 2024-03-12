#include "./packet.hpp"

X_BEGIN

static_assert(std::is_standard_layout_v<xPacketHeader>);
static_assert(sizeof(xPacketHeader) == PacketHeaderSize);
static_assert(sizeof(xPacketHeader) == xPacketHeader::Size);

size_t xPacket::MakeRegisterDispatcherConsumer(void * PacketBuffer, size_t PacketBufferSize, const xPacketCommandId * CmdIds, size_t Total) {
	assert(Total < 1024 && Total <= MaxDispatchableCommandIdCount);
	size_t TotalRequired = PacketHeaderSize + Total * 2;
	if (PacketBufferSize < TotalRequired) {
		return 0;
	}
	xPacketHeader Header;
	Header.CommandId  = xPacketHeader::CmdId_InnernalRequest;
	Header.RequestId  = xPacketHeader::InternalRequest_RegisterDispatcherConsumer;
	Header.PacketSize = TotalRequired;
	Header.Serialize(PacketBuffer);

	auto W = xStreamWriter(PacketBuffer);
	W.Skip(PacketHeaderSize);

	[[maybe_unused]] auto Last = xPacketCommandId(0);
	for (size_t I = 0; I < Total; ++I) {
		auto CmdId = CmdIds[I];
		assert(CmdId <= MaxDispatchableCommandId);
		assert(I == 0 || Last < CmdId);
		Last = CmdId;
		W.W2L((uint16_t)CmdId);
	}
	return TotalRequired;
}

std::vector<xPacketCommandId> xPacket::ParseRegisterDispatcherConsumer(const void * PayloadPtr, size_t PayloadSize) {
	auto   Ret   = std::vector<xPacketCommandId>();
	size_t Total = PayloadSize / 2;
	Ret.resize(Total);
	auto Last = xPacketCommandId(0);
	auto R    = xStreamReader(PayloadPtr);
	for (size_t I = 0; I < Total; ++I) {
		auto & CmdId = Ret[I];
		CmdId        = (xPacketCommandId)R.R2L();
		if (I && CmdId <= Last) {
			return {};
		}
		Last = CmdId;
	}
	return Ret;
}

size_t xPacket::MakeRegisterDispatcherObserver(void * PacketBuffer, size_t PacketBufferSize, const xPacketCommandId * CmdIds, size_t Total) {
	assert(Total < 1024 && Total <= MaxDispatchableCommandIdCount);
	size_t TotalRequired = PacketHeaderSize + Total * 2;
	if (PacketBufferSize < TotalRequired) {
		return 0;
	}
	xPacketHeader Header;
	Header.CommandId  = xPacketHeader::CmdId_InnernalRequest;
	Header.RequestId  = xPacketHeader::InternalRequest_RegisterDispatcherObserver;
	Header.PacketSize = TotalRequired;
	Header.Serialize(PacketBuffer);

	auto W = xStreamWriter(PacketBuffer);
	W.Skip(PacketHeaderSize);

	[[maybe_unused]] auto Last = xPacketCommandId(0);
	for (size_t I = 0; I < Total; ++I) {
		auto CmdId = CmdIds[I];
		assert(CmdId <= MaxDispatchableCommandId);
		assert(I == 0 || Last < CmdId);
		Last = CmdId;
		W.W2L((uint16_t)CmdId);
	}
	return TotalRequired;
}

std::vector<xPacketCommandId> xPacket::ParseRegisterDispatcherObserver(const void * PayloadPtr, size_t PayloadSize) {
	auto   Ret   = std::vector<xPacketCommandId>();
	size_t Total = PayloadSize / 2;
	Ret.resize(Total);
	auto Last = xPacketCommandId(0);
	auto R    = xStreamReader(PayloadPtr);
	for (size_t I = 0; I < Total; ++I) {
		auto & CmdId = Ret[I];
		CmdId        = (xPacketCommandId)R.R2L();
		if (I && CmdId <= Last) {
			return {};
		}
		Last = CmdId;
	}
	return Ret;
}
X_END
