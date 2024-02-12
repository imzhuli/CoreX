#pragma once

#include "../core/core_byte.hpp"
#include "../core/core_min.h"

#include <algorithm>
#include <cstring>

X_BEGIN

using xPacketSize      = uint32_t;
using xPacketCommandId = uint32_t;
using xPacketRequestId = uint64_t;

static constexpr const size_t   PacketHeaderSize = 16u;
static constexpr const uint32_t PacketMagicMask  = 0xFF'000000u;
static constexpr const uint32_t PacketMagicValue = 0xCD'000000u;
static constexpr const uint32_t PacketSizeMask   = 0x00'FFFFFFu;

static constexpr const size_t InvalidPacketSize    = static_cast<size_t>(-1);
static constexpr const size_t MaxPacketSize        = 4096 & PacketSizeMask;
static constexpr const size_t MaxPacketPayloadSize = MaxPacketSize - PacketHeaderSize;

/***
	@brief Such class is a 'almost' direct mapping to stream data header.
	@note  Serialization uses Little-Endian
*/
struct xPacketHeader final {
	static constexpr const size_t           Size                             = 2 * sizeof(uint32_t) + sizeof(uint64_t);
	static constexpr const xPacketCommandId CmdId_InnernalRequest            = 0x00;
	static constexpr const xPacketRequestId InternalRequest_KeepAlive        = 0x00;
	static constexpr const xPacketRequestId InternalRequest_RequestKeepAlive = static_cast<uint64_t>(-1);

	xPacketSize      PacketSize = 0;  // header size included, lower 24 bits as length, higher 8 bits as a magic check
	xPacketCommandId CommandId  = 0;
	xPacketRequestId RequestId  = 0;

	X_INLINE void Serialize(void * DestPtr) const {
		xStreamWriter S(DestPtr);
		S.W4L(MakeHeaderLength(PacketSize));
		S.W4L(CommandId);
		S.W8L(RequestId);
	}

	X_INLINE void Deserialize(const void * SourcePtr) {
		xStreamReader S(SourcePtr);
		PacketSize = PickPackageLength(S.R4L());
		CommandId  = S.R4L();
		RequestId  = S.R8L();
	}

	X_INLINE size_t GetPayloadSize() const {
		return PacketSize - PacketHeaderSize;
	}

	X_INLINE operator bool() const {
		return PacketSize;
	}

	X_STATIC_INLINE xPacketHeader Parse(void * PacketPtr) {
		xPacketHeader Header;
		Header.Deserialize(PacketPtr);
		return Header;
	}

	X_STATIC_INLINE void PatchRequestId(void * PacketPtr, xPacketRequestId RequestId) {
		xStreamWriter S(PacketPtr);
		S.Skip(8 /* PacketSize + CommandId */);
		S.W8L(RequestId);
	};

	X_STATIC_INLINE size_t MakeKeepAlive(void * PackageHeaderBuffer) {
		xPacketHeader Header;
		Header.CommandId  = CmdId_InnernalRequest;
		Header.RequestId  = InternalRequest_KeepAlive;
		Header.PacketSize = PacketHeaderSize;
		Header.Serialize(PackageHeaderBuffer);
		return PacketHeaderSize;
	}

	X_STATIC_INLINE size_t MakeRequestKeepAlive(void * PackageHeaderBuffer) {
		xPacketHeader Header;
		Header.CommandId  = CmdId_InnernalRequest;
		Header.RequestId  = InternalRequest_RequestKeepAlive;
		Header.PacketSize = PacketHeaderSize;
		Header.Serialize(PackageHeaderBuffer);
		return PacketHeaderSize;
	}

	X_INLINE bool IsInternalRequest() const {
		return CommandId == CmdId_InnernalRequest;
	}
	X_INLINE bool IsKeepAlive() const {
		return IsInternalRequest() && RequestId == InternalRequest_KeepAlive;
	}
	X_INLINE bool IsRequestKeepAlive() const {
		return IsInternalRequest() && RequestId == InternalRequest_RequestKeepAlive;
	}

private:
	X_STATIC_INLINE uint32_t MakeHeaderLength(uint32_t PacketSize) {
		assert(PacketSize <= PacketSizeMask);
		return PacketSize | PacketMagicValue;
	}
	X_STATIC_INLINE uint32_t PickPackageLength(uint32_t PacketLengthField) {
		uint32_t PacketSize = PacketLengthField ^ PacketMagicValue;
		return X_LIKELY(PacketSize <= PacketSizeMask) ? PacketSize : 0;
	}
};

struct xPacket {
	X_STATIC_INLINE ubyte * GetPayload(void * PacketPtr) {
		return (ubyte *)PacketPtr + xPacketHeader::Size;
	}
	X_STATIC_INLINE const ubyte * GetPayload(const void * PacketPtr) {
		return (const ubyte *)PacketPtr + xPacketHeader::Size;
	}
};

X_END
