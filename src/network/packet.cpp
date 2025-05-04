#include "./packet.hpp"

#include "../core/core_stream.hpp"

X_BEGIN

static_assert(std::is_standard_layout_v<xPacketHeader>);
static_assert(sizeof(xPacketHeader) == PacketHeaderSize);
static_assert(sizeof(xPacketHeader) == xPacketHeader::Size);

size_t BuildPacket(void * Buffer, size_t BufferSize, xPacketCommandId CommandId, xPacketRequestId RequestId, const void * PayloadPtr, size_t PayloadSize) {
	auto PacketSize = PacketHeaderSize + PayloadSize;
	if (BufferSize < PacketSize) {
		return 0;
	}
	auto Header = xPacketHeader{
		.PacketSize = (xPacketSize)PacketSize,
		.CommandId  = CommandId,
		.RequestId  = RequestId,
	};
	auto Writer = xStreamWriter(Buffer);
	Header.Serialize(Writer.Skip(PacketHeaderSize));
	Writer.W(PayloadPtr, PayloadSize);
	return PacketSize;
}

X_END
