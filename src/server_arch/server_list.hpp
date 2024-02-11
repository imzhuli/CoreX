#pragma once
#include "./base.hpp"

#include <vector>

X_BEGIN

class iServerInfoService {
public:
	virtual bool AddServerInfo(const xServerInfo & ServerInfo) = 0;
	virtual void RemoveServerInfo(const xServerId & ServerId)  = 0;
	virtual auto Get() const -> const xServerInfo *            = 0;
	virtual void Clear()                                       = 0;
};

class xServerInfoServiceSmall
	: public iServerInfoService
	, xNonCopyable {
public:
	X_PRIVATE_MEMBER bool AddServerInfo(const xServerInfo & ServerInfo) override;
	X_PRIVATE_MEMBER void RemoveServerInfo(const xServerId & ServerId) override;
	X_PRIVATE_MEMBER auto Get() const -> const xServerInfo * override;
	X_PRIVATE_MEMBER void Clear() override;

private:
	std::vector<xServerInfo> ServerList;
};

X_END
