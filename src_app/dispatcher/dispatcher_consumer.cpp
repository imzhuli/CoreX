#include "./dispatcher.hpp"

#include <bitset>
#include <cinttypes>
#include <core/string.hpp>
#include <cstdint>

X_BEGIN

bool xDispatcherConsumerService::Init(const xDispatcherConsumerOptions & Options) {
	if (!xService::Init(Options.IoCtxPtr, Options.BindAddress, Options.ConnetionPoolSize, true)) {
		return false;
	}
	for (auto & CG : ConnectionGroups) {
		Renew(CG);
	}
	ConnectionInfoPool.resize(Options.ConnetionPoolSize);
	DispatcherPtr = Options.Outer;
	return true;
}

void xDispatcherConsumerService::Clean() {
	ConnectionInfoPool.clear();
	for (auto & CG : ConnectionGroups) {
		Reset(CG);
	}
	xService::Clean();
}

void xDispatcherConsumerService::OnClientConnected(xServiceClientConnection & Connection) {
	auto   Index          = xIndexId(Connection.GetConnectionId()).GetIndex();
	auto & ConnectionInfo = ConnectionInfoPool[Index];
	Reset(ConnectionInfo);
}

void xDispatcherConsumerService::OnClientClose(xServiceClientConnection & Connection) {
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

bool xDispatcherConsumerService::OnPacket(xServiceClientConnection & Connection, const xPacketHeader & Header, ubyte * PayloadPtr, size_t PayloadSize) {
	auto   ConnectionId   = Connection.GetConnectionId();
	auto   Index          = xIndexId(ConnectionId).GetIndex();
	auto & ConnectionInfo = ConnectionInfoPool[Index];
	X_DEBUG_PRINTF("ConsumerPacket: %" PRIx64 "\n%s", ConnectionId, HexShow(PayloadPtr, PayloadSize).c_str());
	if (!ConnectionInfo.InterestedCommandIdCount) {  // first packet, registering consumer
		if (!Header.IsRegisterDispatcherConsumer() || !PayloadSize) {
			return false;
		}
		auto InterestedCommandIds = xPacket::ParseRegisterDispatcherConsumer(PayloadPtr, PayloadSize);
		X_DEBUG_PRINTF("New Consumer accepted: Interested command number: %zi", InterestedCommandIds.size());
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

	if (Header.IsInternalRequest()) {
		X_DEBUG_PRINTF("Observe should not post internal messages after initialization");
		return false;
	}
	auto PacketPtr  = xPacket::GetPacketPtr(PayloadPtr);
	auto PacketSize = xPacket::GetPacketSize(PayloadSize);
	DispatcherPtr->PostResponse(Header, PacketPtr, PacketSize);
	return true;
}

void xDispatcherConsumerService::DispatchRequest(xPacketCommandId CommandId, const void * PacketPtr, size_t PacketSize) {
	assert(CommandId <= MaxDispatchableCommandId);  // must be checked by caller (Dispatcher)
	auto & Group    = ConnectionGroups[CommandId];
	auto & GroupIds = Group.ConnectionIds;
	if (GroupIds.empty()) {
		X_DEBUG_PRINTF("Empty command id group CommandId=%" PRIu32 " ", CommandId);
		return;
	}
	if (++Group.MostRecentUsedConnectionIndex >= GroupIds.size()) {
		Group.MostRecentUsedConnectionIndex = 0;
	}
	auto PostConnectionId = GroupIds[Group.MostRecentUsedConnectionIndex];
	X_DEBUG_PRINTF("Dispatch to %" PRIx64 "", PostConnectionId);

	PostData(PostConnectionId, PacketPtr, PacketSize);
}

X_END
