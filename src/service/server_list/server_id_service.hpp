#pragma once
#include "../../core/list.hpp"
#include "../../core/memory_pool.hpp"
#include "../../server_arch/tcp_service.hpp"
#include "../base/server_id_manager.hpp"

#include <limits>

X_SERVICE_BEGIN

class xServerIdService final {
public:
	static constexpr const size_t MAX_GROUP_INDEX	  = size_t(std::numeric_limits<xServerGroup>::max());
	static constexpr const size_t MAX_GROUP_COUNT	  = MAX_GROUP_INDEX + 1;
	static constexpr const size_t MAX_SERVER_ID_COUNT = xServerIdManager::MAX_SERVER_ID_COUNT;
	static constexpr const size_t MAX_SERVER_ID_INDEX = xServerIdManager::MAX_SERVER_ID_INDEX;

private:
	using xIdManagerList = std::array<xServerIdManager *, MAX_GROUP_COUNT>;
	struct xConnectionContext : xListNode {
		uint64_t						  StartupTimestampMS = 0;
		uint64_t						  ServerId			 = 0;
		xTcpServiceClientConnectionHandle ConnectionHandle	 = {};
	};
	using xConnectionContextList = xList<xConnectionContext>;

public:
	X_API_MEMBER bool Init(xIoContext * ICP, const xNetAddress & BindAddress);
	X_API_MEMBER void Clean();
	X_API_MEMBER void Tick(uint64_t NowMS);

	X_API_MEMBER bool EnableServerGroup(xServerGroup Type);
	X_API_MEMBER void DisableServerGroup(xServerGroup Type);

	using xOnNewServerId	= std::function<void(xServerId Id, const xNetAddress & ExportAddress)>;
	using xOnRemoveServerId = std::function<void(xServerId Id)>;

	xOnNewServerId	  OnNewServerId	   = Noop<>;
	xOnRemoveServerId OnRemoveServerId = Noop<>;

private:
	X_API_MEMBER auto GetServerIdManagerByServerId(uint64_t Serverid) -> xServerIdManager *;
	X_API_MEMBER void KillUnregisteredClientConnections();
	X_API_MEMBER void OnNewClientConnection(const xTcpServiceClientConnectionHandle & Handle);
	X_API_MEMBER void OnCleanClientConnection(const xTcpServiceClientConnectionHandle & Handle);
	X_API_MEMBER bool OnClientConnectionPacket(const xTcpServiceClientConnectionHandle & Handle, xPacketCommandId CommandId, xPacketRequestId, ubyte * Payload, size_t PayloadSize);

private:
	xTicker							LocalTicker;
	xTcpService						TcpService;
	xIdManagerList *				IdManagerList = nullptr;
	xMemoryPool<xConnectionContext> ConnectionContextPool;
	xConnectionContextList			TimeoutContextList;
};

X_SERVICE_END
