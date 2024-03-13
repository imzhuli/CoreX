#include "./dispatcher.hpp"

#include <cinttypes>
#include <core/string.hpp>
#include <cstdint>

X_BEGIN

bool xDispatcherProducerService::Init(const xDispatcherProducerOptions & Options) {
	if (!xService::Init(Options.IoCtxPtr, Options.BindAddress, true)) {
		return false;
	}
	DispatcherPtr = Options.Outer;
	return true;
}

void xDispatcherProducerService::Clean() {
	xService::Clean();
}

bool xDispatcherProducerService::OnPacket(xServiceClientConnection & Connection, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) {
	X_DEBUG_PRINTF("ProducerPacket: \n%s", HexShow(PayloadPtr, PayloadSize).c_str());
	assert(!Header.IsInternalRequest());

	auto PacketPtr  = xPacket::GetPacketPtr(PayloadPtr);
	auto PacketSize = xPacket::GetPacketSize(PayloadSize);
	DispatcherPtr->PostRequest(Connection.GetConnectionId(), Header, PacketPtr, PacketSize);
	return true;
}

X_END
