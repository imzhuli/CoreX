#include "./broadcaster.hpp"

#include <cinttypes>
#include <core/string.hpp>
#include <cstdint>

X_BEGIN

bool xBroadcasterProducerService::Init(const xBroadcasterProducerOptions & Options) {
	if (!xService::Init(Options.IoCtxPtr, Options.BindAddress, true)) {
		return false;
	}
	BroadcasterPtr = Options.Outer;
	return true;
}

void xBroadcasterProducerService::Clean() {
	xService::Clean();
}

bool xBroadcasterProducerService::OnPacket(xServiceClientConnection & Connection, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) {
	X_DEBUG_PRINTF("ProducerPacket: \n%s", HexShow(PayloadPtr, PayloadSize).c_str());
	if (Header.IsInternalRequest() || Header.CommandId > MaxDispatchableCommandId) {
		return false;
	}

	auto PacketPtr  = xPacket::GetPacketPtr(PayloadPtr);
	auto PacketSize = xPacket::GetPacketSize(PayloadSize);
	BroadcasterPtr->Broadcast(Connection.GetConnectionId(), Header, PacketPtr, PacketSize);
	return true;
}

X_END
