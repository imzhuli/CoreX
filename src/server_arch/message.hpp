#pragma once
#include "../core/core_min.hpp"
#include "../network/packet.hpp"
#include "./message_reader.hpp"
#include "./message_writer.hpp"

#include <limits>

X_BEGIN

class xBinaryMessage
	: protected xBinaryMessageReader
	, protected xBinaryMessageWriter {

public:
	X_API_MEMBER size_t Serialize(void * Dst, size_t Size);
	X_API_MEMBER size_t Deserialize(const void * Src, size_t Size);

protected:
	X_API_MEMBER virtual void SerializeMembers();
	X_API_MEMBER virtual void DeserializeMembers();

	X_INLINE xBinaryMessageReader * GetReader() { return this; }
	X_INLINE xBinaryMessageWriter * GetWriter() { return this; }
};

X_API size_t WritePacket(xPacketCommandId CmdId, xPacketRequestId RequestId, void * Buffer, size_t BufferSize, xBinaryMessage & Message);

X_END
