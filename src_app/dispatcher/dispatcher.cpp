#include "./dispatcher.hpp"

#include <cinttypes>
#include <core/core_time.hpp>
#include <core/string.hpp>
#include <cstdint>

X_BEGIN

static constexpr const uint64_t REQUEST_TIMEOUT_MS = 3'000;

/* dispatcher inner classes */
std::string xDispatcherService::ToString(const xRequestMapping & RM) {
	char Buffer[128];
	snprintf(
		Buffer, SafeLength(Buffer), "@%" PRIu64 ", ConnectionId=%0" PRIx64 ", SourceRequestId=%0" PRIx64 ", ConsumerRequestId=%0" PRIx64 "", RM.TimestampMS / 1000,
		RM.SourceConnectionId, RM.SourceRequestId, RM.ConsumerRequestId
	);
	LastOf(Buffer) = '\0';
	return std::string(Buffer);
}

/* dispatcher */
bool xDispatcherService::Init(const xDispatcherOptions & Options) {
	if (!IoCtx.Init()) {
		return false;
	}
	auto IG = MakeResourceCleaner(IoCtx);

	auto PO = xDispatcherProducerOptions{
		.Outer             = this,
		.IoCtxPtr          = &IoCtx,
		.BindAddress       = Options.ProducerAddress,
		.ConnetionPoolSize = 1024,
	};
	if (!ProducerService.Init(PO)) {
		return false;
	}
	auto PG = MakeResourceCleaner(ProducerService);

	auto CO = xDispatcherConsumerOptions{
		.Outer             = this,
		.IoCtxPtr          = &IoCtx,
		.BindAddress       = Options.ConsumerAddress,
		.ConnetionPoolSize = 1024,
	};
	if (!ConsumerService.Init(CO)) {
		return false;
	}
	auto CG = MakeResourceCleaner(ConsumerService);

	if (!RequestIdPool.Init(Options.RequestIdPoolSize)) {
		return false;
	}
	auto RG = MakeResourceCleaner(RequestIdPool);

	Dismiss(IG, PG, CG, RG);
	return true;
}

void xDispatcherService::Clean() {
	MakeResourceCleaner(IoCtx, ProducerService, ConsumerService, RequestIdPool);
}

void xDispatcherService::Tick() {
	NowMS = GetTimestampMS();
	IoCtx.LoopOnce();
	ProducerService.Tick(NowMS);
	ConsumerService.Tick(NowMS);
	auto KillTimestamp = NowMS - REQUEST_TIMEOUT_MS;
	for (auto & N : RequestTimeoutList) {
		if (SignedDiff(KillTimestamp, N.TimestampMS) < 0) {
			break;
		}
		assert(N.ConsumerRequestId);
		assert(RequestIdPool.CheckAndGet(N.ConsumerRequestId) == &N);
		X_DEBUG_PRINTF("@%" PRIu64 ", Release timeout request: %s", NowMS / 1000, ToString(N).c_str());
		RequestIdPool.Release(N.ConsumerRequestId);
	}
}

void xDispatcherService::PostRequest(uint64_t ProducerConnectionId, const xPacketHeader & Header, ubyte * PacketPtr, size_t PacketSize) {
	if (Header.RequestId) {  // won't expect response if request id is zero
		auto ConsumerRequestId = RequestIdPool.Acquire();
		if (!ConsumerRequestId) {
			X_DEBUG_PRINTF("Failed to allocate consumer request id");
			return;
		}
		auto & Node             = RequestIdPool[ConsumerRequestId];
		Node.TimestampMS        = NowMS;
		Node.SourceConnectionId = ProducerConnectionId;
		Node.SourceRequestId    = Header.RequestId;
		Node.ConsumerRequestId  = ConsumerRequestId;
		RequestTimeoutList.AddTail(Node);
		xPacketHeader::PatchRequestId(PacketPtr, ConsumerRequestId);
	}
	ConsumerService.DispatchRequest(Header.CommandId, PacketPtr, PacketSize);
	X_DEBUG_PRINTF("Dispatching request: \n%s", HexShow(PacketPtr, PacketSize).c_str());
}

void xDispatcherService::PostResponse(const xPacketHeader & Header, ubyte * PacketPtr, size_t PacketSize) {
	auto RequestId = Header.RequestId;
	auto NodePtr   = RequestIdPool.CheckAndGet(RequestId);
	if (!NodePtr) {
		X_DEBUG_PRINTF("Invalid RequestId: %" PRIx64 "", RequestId);
		return;
	}
	xPacketHeader::PatchRequestId(PacketPtr, NodePtr->SourceRequestId);
	ProducerService.PostData(NodePtr->SourceConnectionId, PacketPtr, PacketSize);
	RequestIdPool.Release(RequestId);
}

X_END
