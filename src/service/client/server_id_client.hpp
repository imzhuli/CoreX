#pragma once
#include "../../server_arch/tcp_client_wrapper.hpp"
#include "../base/_.hpp"

X_SERVICE_BEGIN

extern uint64_t LoadLocalServerId(const std::string & LocalServerIdFilename);
extern void		DumpLocalServerId(const std::string & LocalServerIdFilename, uint64_t LocalServerId);

struct xServerIdClientOptions final {
	xServerType ServerType		 = 0;
	uint64_t	PreviousServerId = 0;
	xNetAddress ExportAddress	 = {};
};

class xServerIdClient final {
public:
	X_API_MEMBER bool Init(xIoContext * ICP, const xServerIdClientOptions & Options, const xNetAddress & ServerIdCenterAddress);
	X_API_MEMBER void Clean();
	X_API_MEMBER void Tick(uint64_t NowMS);

	X_INLINE uint64_t GetLocalServerId() const { return LocalServerIdDirty ? 0 : LocalServerId; }

	std::function<void(uint64_t)> OnServerIdUpdated = Noop<>;

private:
	void OnServerConnected();
	bool OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize);

private:
	xTcpClientWrapper ClientWrapper;
	xServerType		  ServerType		 = 0;
	xNetAddress		  ExportAddress		 = {};
	uint64_t		  LocalServerId		 = 0;
	bool			  LocalServerIdDirty = false;
	std::string		  LocalServerIdFilename;
};

X_SERVICE_END
