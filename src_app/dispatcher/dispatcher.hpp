#pragma once
#include <core/core_min.hpp>
#include <server_arch/service.hpp>

X_BEGIN

class xDispatcherService;
class xDispatcherProducerService;
class xDispatcherConsumerService;

// Producers

struct xDispatcherProducerOptions {
	xDispatcherService * Outer;
	xIoContext *         IoCtxPtr;
	xNetAddress          BindAddress;
	size_t               ConnetionPoolSize;
};

class xDispatcherProducerService final : xService {
public:
	bool Init(const xDispatcherProducerOptions & Options);
	void Clean();

	using xService::PostData;
	using xService::Tick;

protected:
	bool OnPacket(xServiceClientConnection & Connection, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) override;

private:
	xDispatcherService * DispatcherPtr;
};

// Consumer
struct xDispatcherConsumerOptions final {
	xDispatcherService * Outer;
	xIoContext *         IoCtxPtr;
	xNetAddress          BindAddress;
	size_t               ConnetionPoolSize;
};

class xDispatcherConsumerService final : xService {
public:
	bool Init(const xDispatcherConsumerOptions & Options);
	void Clean();
	void DispatchRequest(xPacketCommandId CommandId, const void * PacketPtr, size_t PacketSize);
	using xService::Tick;

protected:
	void OnClientConnected(xServiceClientConnection & Connection) override;
	void OnClientClose(xServiceClientConnection & Connection) override;
	bool OnPacket(xServiceClientConnection & Connection, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) override;

private:
	struct xConnectionGroupByCommandId {
		std::vector<uint64_t> ConnectionIds;
		size_t                MostRecentUsedConnectionIndex = 0;
	};

	struct xConnectionInfo {
		uint16_t InterestedCommandIds[MaxDispatchableCommandIdCount];
		size_t   InterestedCommandIdCount = 0;
	};

	xDispatcherService *         DispatcherPtr;
	xConnectionGroupByCommandId  ConnectionGroups[MaxDispatchableCommandIdCount];
	std::vector<xConnectionInfo> ConnectionInfoPool;
};

// Dispatchers
struct xDispatcherOptions final {
	xNetAddress ProducerAddress;
	xNetAddress ConsumerAddress;
	size_t      RequestIdPoolSize = 10240;
};

class xDispatcherService final {
public:
	bool Init(const xDispatcherOptions & Options);
	void Clean();
	void Tick();

protected:
	friend class xDispatcherProducerService;
	friend class xDispatcherConsumerService;

	void PostRequest(uint64_t ProducerConnectionId, const xPacketHeader & Header, ubyte * PacketPtr, size_t PacketSize);
	void PostResponse(const xPacketHeader & Header, ubyte * PacketPtr, size_t PacketSize);

private:
	struct xRequestMapping : xListNode {
		uint64_t TimestampMS;
		uint64_t SourceConnectionId;
		uint64_t SourceRequestId;
		uint64_t ConsumerRequestId;  // key from RequestIdPool, for debug use
	};
	static std::string ToString(const xRequestMapping & RM);
	using xRequestTimeoutList = xList<xRequestMapping>;

private:
	xIoContext                       IoCtx;
	int64_t                          NowMS;
	xDispatcherProducerService       ProducerService;
	xDispatcherConsumerService       ConsumerService;
	xIndexedStorage<xRequestMapping> RequestIdPool;
	xRequestTimeoutList              RequestTimeoutList;
};

X_END
