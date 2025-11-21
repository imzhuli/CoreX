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

	using xOnConnected    = std::function<void()>;
	using xOnDisconnected = std::function<void()>;
	using xOnPacket       = std::function<bool(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize)>;

	xOnConnected    OnConnected    = Noop<>;
	xOnDisconnected OnDisconnected = Noop<>;
	xOnPacket       OnPacket       = Noop<true>;

private:
	X_API_MEMBER void OnServerConnected() override;
	X_API_MEMBER void OnServerClose() override;
	X_API_MEMBER bool OnServerPacket(xPacketCommandId CommandId, xPacketRequestId RequestId, ubyte * PayloadPtr, size_t PayloadSize) override;

private:
	bool         HasInstance = false;
	xIoContext * ICP         = nullptr;
};

X_END
