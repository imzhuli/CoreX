#include "./broadcaster.hpp"

#include <bitset>
#include <cinttypes>
#include <core/string.hpp>
#include <cstdint>

X_BEGIN

bool xBroadcasterObserverService::Init(const xBroadcasterObserverOptions & Options) {
	if (!xService::Init(Options.IoCtxPtr, Options.BindAddress, Options.ConnetionPoolSize, true)) {
		return false;
	}
	for (auto & CG : ConnectionGroups) {
		Renew(CG);
	}
	ConnectionInfoPool.resize(Options.ConnetionPoolSize);
	BroadcasterPtr = Options.Outer;
	return true;
}

void xBroadcasterObserverService::Clean() {
	ConnectionInfoPool.clear();
	for (auto & CG : ConnectionGroups) {
		Reset(CG);
	}
	xService::Clean();
}

void xBroadcasterObserverService::OnClientConnected(xServiceClientConnection & Connection) {
	auto   Index          = xIndexId(Connection.GetConnectionId()).GetIndex();
	auto & ConnectionInfo = ConnectionInfoPool[Index];
	Reset(ConnectionInfo);
}

void xBroadcasterObserverService::OnClientClose(xServiceClientConnection & Connection) {
	X_DEBUG_PRINTF("ConnectionClose: %" PRIx64 "", Connection.GetConnectionId());
	auto   ConnectionId   = Connection.GetConnectionId();
	auto   Index          = xIndexId(ConnectionId).GetIndex();
	auto & ConnectionInfo = ConnectionInfoPool[Index];
	if (!ConnectionInfo.InterestedCommandIdCount) {  // first packet, registering consumer
		return;
	}
	for (size_t I = 0; I < ConnectionInfo.InterestedCommandIdCount; ++I) {
		auto   CmdId = ConnectionInfo.InterestedCommandIds[I];
		auto & Group = ConnectionGroups[CmdId];
		for (auto J = Group.ConnectionIds.begin(); J != Group.ConnectionIds.end(); ++J) {
			if (*J == ConnectionId) {
				Group.ConnectionIds.erase(J);
				break;
			}
		}
	}
}

bool xBroadcasterObserverService::OnPacket(xServiceClientConnection & Connection, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) {
	auto   ConnectionId   = Connection.GetConnectionId();
	auto   Index          = xIndexId(ConnectionId).GetIndex();
	auto & ConnectionInfo = ConnectionInfoPool[Index];
	X_DEBUG_PRINTF("ObserverPacket: %" PRIx64 "\n%s", ConnectionId, HexShow(PayloadPtr, PayloadSize).c_str());
	if (!ConnectionInfo.InterestedCommandIdCount) {  // first packet, registering consumer
		if (!Header.IsRegisterDispatcherObserver() || !PayloadSize) {
			return false;
		}
		auto InterestedCommandIds = xPacket::ParseRegisterDispatcherObserver(PayloadPtr, PayloadSize);
		X_DEBUG_PRINTF("New Observer accepted: Interested command number: %zi", InterestedCommandIds.size());
		if (InterestedCommandIds.empty()) {
			return false;
		}
		for (auto CmdId : InterestedCommandIds) {
			X_DEBUG_PRINTF("Adding interested commandId: %" PRIx32 "", CmdId);
			auto & Group = ConnectionGroups[CmdId];
			Group.ConnectionIds.push_back(ConnectionId);
			ConnectionInfo.InterestedCommandIds[ConnectionInfo.InterestedCommandIdCount++] = CmdId;
		}
		return true;
	}
	X_DEBUG_PRINTF("Observe should not post messages after initialization");
	return false;
}

void xBroadcasterObserverService::Broadcast(xPacketCommandId CommandId, const void * PacketPtr, size_t PacketSize) {
	assert(CommandId <= MaxDispatchableCommandId);  // must be checked by caller (Broadcaster)
	auto & Group    = ConnectionGroups[CommandId];
	auto & GroupIds = Group.ConnectionIds;

	for (auto ConnectionId : GroupIds) {
		PostData(ConnectionId, PacketPtr, PacketSize);
	}
}

X_END
