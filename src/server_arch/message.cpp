#include "./message.hpp"

X_BEGIN

size_t xBinaryMessage::Serialize(void * Dst, size_t Size) {
	auto SSize = static_cast<ssize_t>(Size);
	assert(SSize < std::numeric_limits<ssize_t>::max());

	auto FlagGuard = xValueGuard(IsSerializing, true);
	xBinaryMessageWriter::Reset(Dst, SSize);
	SerializeMembers();
	auto Consumed = xBinaryMessageWriter::GetConsumedSize();
#ifndef NDEBUG
	if (!Consumed && !xBinaryMessageWriter::HasError()) {
		X_PFATAL("zero length object is not allowd");
	}
#endif
	return Consumed;
}

size_t xBinaryMessage::Deserialize(const void * Src, size_t Size) {
	auto SSize = static_cast<ssize_t>(Size);
	assert(SSize < std::numeric_limits<ssize_t>::max());

	auto FlagGuard = xValueGuard(IsDeserializing, true);
	xBinaryMessageReader::Reset(Src, SSize);
	DeserializeMembers();
	auto Consumed = xBinaryMessageReader::GetConsumedSize();
#ifndef NDEBUG
	if (!Consumed && !xBinaryMessageReader::HasError()) {
		X_PFATAL("zero length object is not allowd");
	}
#endif
	return Consumed;
}

void xBinaryMessage::SerializeMembers() {
	xBinaryMessageWriter::SetError();
}

void xBinaryMessage::DeserializeMembers() {
	xBinaryMessageReader::SetError();
}

/****************/
size_t WritePacket(xPacketCommandId CmdId, xPacketRequestId RequestId, void * Buffer, size_t BufferSize, xBinaryMessage & Message) {
	assert(BufferSize >= PacketHeaderSize);
	auto PayloadPtr     = static_cast<ubyte *>(Buffer) + PacketHeaderSize;
	auto MaxPayloadSize = BufferSize - PacketHeaderSize;
	auto PayloadSize    = Message.Serialize(PayloadPtr, MaxPayloadSize);
	if (InvalidDataSize == PayloadSize) {
		return 0;
	}

	auto Header       = xPacketHeader();
	Header.PacketSize = PacketHeaderSize + PayloadSize;
	Header.CommandId  = CmdId;
	Header.RequestId  = RequestId;
	Header.Serialize(Buffer);
	return Header.PacketSize;
}

X_END
