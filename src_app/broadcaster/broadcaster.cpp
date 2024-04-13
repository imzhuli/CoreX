#include "./broadcaster.hpp"

#include <cinttypes>
#include <core/core_time.hpp>
#include <core/string.hpp>
#include <cstdint>

X_BEGIN

/* Broadcaster */
bool xBroadcasterService::Init(const xBroadcasterOptions & Options) {
	if (!IoCtx.Init()) {
		return false;
	}
	auto IG = MakeResourceCleaner(IoCtx);

	auto PO = xBroadcasterProducerOptions{
		.Outer             = this,
		.IoCtxPtr          = &IoCtx,
		.BindAddress       = Options.ProducerAddress,
		.ConnetionPoolSize = 1024,
	};
	if (!ProducerService.Init(PO)) {
		return false;
	}
	auto PG = MakeResourceCleaner(ProducerService);

	auto CO = xBroadcasterObserverOptions{
		.Outer             = this,
		.IoCtxPtr          = &IoCtx,
		.BindAddress       = Options.ObserverAddress,
		.ConnetionPoolSize = 1024,
	};
	if (!ObserverService.Init(CO)) {
		return false;
	}
	auto CG = MakeResourceCleaner(ObserverService);

	Dismiss(IG, PG, CG);
	return true;
}

void xBroadcasterService::Clean() {
	MakeResourceCleaner(IoCtx, ProducerService, ObserverService);
}

void xBroadcasterService::Tick() {
	NowMS = GetTimestampMS();
	IoCtx.LoopOnce();
	ProducerService.Tick(NowMS);
	ObserverService.Tick(NowMS);
}

void xBroadcasterService::Broadcast(uint64_t ProducerConnectionId, const xPacketHeader & Header, ubyte * PacketPtr, size_t PacketSize) {

	ObserverService.Broadcast(Header.CommandId, PacketPtr, PacketSize);
	X_DEBUG_PRINTF("Dispatching request: \n%s", HexShow(PacketPtr, PacketSize).c_str());
}

X_END
