#pragma once
#include "../core/functional.hpp"
#include "./client.hpp"

X_BEGIN

class xClientWrapper final : private xClient {

public:
	X_API_MEMBER bool Init(xIoContext * ICP);
	X_API_MEMBER bool Init(xIoContext * ICP, const xNetAddress & Address);
	X_API_MEMBER void Clean();
	X_API_MEMBER void UpdateTarget(const xNetAddress & Address);
	X_API_MEMBER void PostMessage(xPacketCommandId CmdId, xPacketRequestId RequestId, xBinaryMessage & Message);
	X_API_MEMBER void Tick(uint64_t NowMS);
	X_API_MEMBER void Kill();

	using xClient::OnServerConnected;
	using xClient::OnServerDisconnected;
	using xClient::OnServerPacket;

private:
	bool         HasInstance = false;
	xIoContext * ICP         = nullptr;
};

X_END
