#pragma once
#include "../core/core_min.hpp"
#include "../network/packet.hpp"
#include "./message_reader.hpp"
#include "./message_writer.hpp"

#include <limits>

X_BEGIN

class xBinaryMessage
	: protected xBinaryMessageWriter
	, protected xBinaryMessageReader {

public:
	X_API_MEMBER size_t Serialize(void * Dst, size_t Size);
	X_API_MEMBER size_t Deserialize(const void * Src, size_t Size);

protected:
	X_API_MEMBER virtual void SerializeMembers();
	X_API_MEMBER virtual void DeserializeMembers();

	X_INLINE xBinaryMessageWriter * GetWriter() { return IsSerializing ? this : nullptr; }
	X_INLINE xBinaryMessageReader * GetReader() { return IsDeserializing ? this : nullptr; }

private:
	bool IsSerializing   = false;
	bool IsDeserializing = false;
};

X_API size_t WriteMessage(void * Buffer, size_t BufferSize, xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);
template <size_t Size>
X_INLINE size_t WriteMessage(ubyte (&Buffer)[Size], xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message) {
	return WriteMessage(Buffer, Size, CmdId, RequestId, Message);
}

X_END
