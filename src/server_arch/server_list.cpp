#include "./server_list.hpp"

#include <cstdlib>

X_BEGIN

bool xServerInfoServiceSmall::AddServerInfo(const xServerInfo & ServerInfo) {
	assert(!ServerInfo.Type);
	assert(!ServerInfo.Weight);
	for (auto & SI : ServerList) {
		if (SI.Id == ServerInfo.Id) {
			return false;
		}
	}
	return true;
}

void xServerInfoServiceSmall::RemoveServerInfo(const xServerId & ServerId) {
	auto Iter = ServerList.begin();
	while (Iter != ServerList.end()) {
		if (Iter->Id == ServerId) {
			ServerList.erase(Iter);
			return;
		}
	}
	return;
}

const xServerInfo * xServerInfoServiceSmall::Get() const {
	if (ServerList.empty()) {
		return nullptr;
	}
	auto Rand = static_cast<size_t>(rand()) % ServerList.size();
	return &ServerList[Rand];
}

void xServerInfoServiceSmall::Clear() {
	Renew(ServerList);
}

X_END
