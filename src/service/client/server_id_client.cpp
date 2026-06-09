#include "./server_id_client.hpp"

#include "../../core/string.hpp"
#include "../internal/id.hpp"
#include "../internal/protocol/register_server.hpp"

X_SERVICE_BEGIN

//////////////////////////////////////////////////////

bool xServerIdClient::Init(xIoContext * ICP, const xServerIdClientOptions & Options, const xNetAddress & ServerIdCenterAddress) {
	if (!ClientWrapper.Init(ICP)) {
		return false;
	}
	ClientWrapper.OnServerConnected = Delegate(&xServerIdClient::OnServerConnected, this);
	ClientWrapper.OnServerPacket	= Delegate(&xServerIdClient::OnServerPacket, this);

	ServerType	  = Options.ServerType;
	ExportAddress = Options.ExportAddress;
	if ((LocalServerId = Options.PreviousServerId)) {
		auto CheckType = ExtractServerType(LocalServerId);
		if (CheckType != ServerType) {
			Reset(LocalServerId);
		}
	}
	ClientWrapper.UpdateTarget(ServerIdCenterAddress);
	return LocalServerIdDirty = true;
}

void xServerIdClient::Clean() {
	Reset(ServerType);
	Reset(ExportAddress);
	Reset(LocalServerId);
	ClientWrapper.Clean();
	ClientWrapper.UpdateTarget({});
}

void xServerIdClient::Tick(uint64_t NowMS) {
	ClientWrapper.Tick(NowMS);
}

void xServerIdClient::OnServerConnected() {
	auto Req			 = xMsg_RegisterServer();
	Req.ServerType		 = ServerType;
	Req.PreviousServerId = LocalServerId;
	Req.ExportAddress	 = ExportAddress;
	ClientWrapper.PostMessage(xPacketHeader::CmdId_InnernalRequest, InternalRequest_RegisterServer, Req);
}

bool xServerIdClient::OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) {
	if (!xPacketHeader::IsInternalRequest(CommandId) || RequestId != InternalRequest_RegisterServerResp) {
		// X_DEBUG_PRINTF("invalid command id");
		return false;
	}

	auto Resp = xMsg_RegisterServerResp();
	if (!Resp.Deserialize(PayloadPtr, PayloadSize)) {
		// X_DEBUG_PRINTF("invalid packet");
		return false;
	}
	if (LocalServerId != Resp.NewServerId) {
		// X_DEBUG_PRINTF("update server id");
		LocalServerId	   = Resp.NewServerId;
		LocalServerIdDirty = true;
	}
	if (Steal(LocalServerIdDirty)) {
		OnServerIdUpdated(LocalServerId);
	}
	return true;
}

X_SERVICE_END
