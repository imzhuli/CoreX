#pragma once

#include "../core/core_min.h"
#include "../core/core_stream.hpp"

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

static constexpr const xPacketCommandId MaxDispatchableCommandId      = 0xFFu;
static constexpr const size_t           MaxDispatchableCommandIdCount = 1 + MaxDispatchableCommandId;

/***
	@brief Such class is a 'almost' direct mapping to stream data header.
	@note  Serialization uses Little-Endian
*/
struct xPacketHeader final {
	static constexpr const size_t           Size                                       = 2 * sizeof(uint32_t) + sizeof(uint64_t);
	static constexpr const xPacketCommandId CmdId_InnernalRequest                      = xPacketCommandId(-1);
	static constexpr const xPacketRequestId InternalRequest_KeepAlive                  = xPacketRequestId(-0);
	static constexpr const xPacketRequestId InternalRequest_RequestKeepAlive           = xPacketRequestId(-1);
	static constexpr const xPacketRequestId InternalRequest_RegisterDispatcherConsumer = xPacketRequestId(-2);
	static constexpr const xPacketRequestId InternalRequest_RegisterDispatcherObserver = xPacketRequestId(-3);

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
		PacketSize = PickPacketLength(S.R4L());
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

	X_STATIC_INLINE size_t MakeKeepAlive(void * PacketHeaderBuffer) {
		xPacketHeader Header;
		Header.CommandId  = CmdId_InnernalRequest;
		Header.RequestId  = InternalRequest_KeepAlive;
		Header.PacketSize = PacketHeaderSize;
		Header.Serialize(PacketHeaderBuffer);
		return PacketHeaderSize;
	}

	X_STATIC_INLINE size_t MakeRequestKeepAlive(void * PacketHeaderBuffer) {
		xPacketHeader Header;
		Header.CommandId  = CmdId_InnernalRequest;
		Header.RequestId  = InternalRequest_RequestKeepAlive;
		Header.PacketSize = PacketHeaderSize;
		Header.Serialize(PacketHeaderBuffer);
		return PacketHeaderSize;
	}

	X_STATIC_INLINE size_t MakeRegisterDispatcherConsumer(void * PacketBuffer, size_t PacketBufferSize, const xPacketCommandId * CmdIds, size_t Total) {
		assert(Total <= MaxDispatchableCommandIdCount);
		size_t TotalRequired = PacketHeaderSize + Total;
		if (PacketBufferSize < TotalRequired) {
			return 0;
		}
		xPacketHeader Header;
		Header.CommandId  = CmdId_InnernalRequest;
		Header.RequestId  = InternalRequest_RegisterDispatcherConsumer;
		Header.PacketSize = TotalRequired;
		Header.Serialize(PacketBuffer);
		auto W = xStreamWriter(PacketBuffer);
		W.Skip(PacketHeaderSize);
		for (size_t I = 0; I < Total; ++I) {
			auto CmdId = CmdIds[I];
			assert(CmdId <= MaxDispatchableCommandId);
			W.W1((uint8_t)CmdId);
		}
		return TotalRequired;
	}

	X_STATIC_INLINE size_t MakeRegisterDispatcherObserver(void * PacketBuffer, size_t PacketBufferSize, const xPacketCommandId * CmdIds, size_t Total) {
		assert(Total <= MaxDispatchableCommandIdCount);
		size_t TotalRequired = PacketHeaderSize + Total;
		if (PacketBufferSize < TotalRequired) {
			return 0;
		}
		xPacketHeader Header;
		Header.CommandId  = CmdId_InnernalRequest;
		Header.RequestId  = InternalRequest_RegisterDispatcherObserver;
		Header.PacketSize = TotalRequired;
		Header.Serialize(PacketBuffer);
		auto W = xStreamWriter(PacketBuffer);
		W.Skip(PacketHeaderSize);
		for (size_t I = 0; I < Total; ++I) {
			auto CmdId = CmdIds[I];
			assert(CmdId <= MaxDispatchableCommandId);
			W.W1((uint8_t)CmdId);
		}
		return TotalRequired;
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
	X_INLINE bool IsRegisterDispatcherConsumer() const {
		return IsInternalRequest() && RequestId == InternalRequest_RegisterDispatcherConsumer;
	}
	X_INLINE bool IsRegisterDispatcherObserver() const {
		return IsInternalRequest() && RequestId == InternalRequest_RegisterDispatcherObserver;
	}

private:
	X_STATIC_INLINE uint32_t MakeHeaderLength(uint32_t PacketSize) {
		assert(PacketSize <= PacketSizeMask);
		return PacketSize | PacketMagicValue;
	}
	X_STATIC_INLINE uint32_t PickPacketLength(uint32_t PacketLengthField) {
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
	X_STATIC_INLINE size_t GetPayloadSize(size_t PacketSize) {
		return PacketSize - PacketHeaderSize;
	}

	X_STATIC_INLINE ubyte * GetPacket(void * PayloedPtr) {
		return (ubyte *)PayloedPtr - xPacketHeader::Size;
	}
	X_STATIC_INLINE const ubyte * GetPacket(const void * PayloedPtr) {
		return (const ubyte *)PayloedPtr - xPacketHeader::Size;
	}
	X_STATIC_INLINE size_t GetPacketSize(size_t PayloadSize) {
		return PayloadSize + PacketHeaderSize;
	}
};

X_END
