#pragma once
#include <core/core_min.hpp>
#include <server_arch/service.hpp>

X_BEGIN

class xBroadcasterService;
class xBroadcasterProducerService;
class xBroadcasterObserverService;

// Producers

struct xBroadcasterProducerOptions {
	xBroadcasterService * Outer;
	xIoContext *          IoCtxPtr;
	xNetAddress           BindAddress;
	size_t                ConnetionPoolSize;
};

class xBroadcasterProducerService final : xService {
public:
	bool Init(const xBroadcasterProducerOptions & Options);
	void Clean();

	using xService::PostData;
	using xService::Tick;

protected:
	bool OnPacket(xServiceClientConnection & Connection, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) override;

private:
	xBroadcasterService * BroadcasterPtr;
};

// Observer
struct xBroadcasterObserverOptions final {
	xBroadcasterService * Outer;
	xIoContext *          IoCtxPtr;
	xNetAddress           BindAddress;
	size_t                ConnetionPoolSize;
};

class xBroadcasterObserverService final : xService {
public:
	bool Init(const xBroadcasterObserverOptions & Options);
	void Clean();
	void Broadcast(xPacketCommandId CommandId, const void * PacketPtr, size_t PacketSize);
	using xService::Tick;

protected:
	void OnClientConnected(xServiceClientConnection & Connection) override;
	void OnClientClose(xServiceClientConnection & Connection) override;
	bool OnPacket(xServiceClientConnection & Connection, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) override;

private:
	struct xConnectionGroupByCommandId {
		std::vector<uint64_t> ConnectionIds;
	};

	struct xConnectionInfo {
		uint16_t InterestedCommandIds[MaxDispatchableCommandIdCount];
		size_t   InterestedCommandIdCount = 0;
	};

	xBroadcasterService *        BroadcasterPtr;
	xConnectionGroupByCommandId  ConnectionGroups[MaxDispatchableCommandIdCount];
	std::vector<xConnectionInfo> ConnectionInfoPool;
};

// Broadcasters
struct xBroadcasterOptions final {
	xNetAddress ProducerAddress;
	xNetAddress ObserverAddress;
};

class xBroadcasterService final {
public:
	bool Init(const xBroadcasterOptions & Options);
	void Clean();
	void Tick();

protected:
	friend class xBroadcasterProducerService;
	friend class xBroadcasterObserverService;

	void Broadcast(uint64_t ProducerConnectionId, const xPacketHeader & Header, ubyte * PacketPtr, size_t PacketSize);

private:
	xIoContext                  IoCtx;
	int64_t                     NowMS;
	xBroadcasterProducerService ProducerService;
	xBroadcasterObserverService ObserverService;
};

X_END
