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
	bool AddServerInfo(const xServerInfo & ServerInfo) override;
	void RemoveServerInfo(const xServerId & ServerId) override;
	auto Get() const -> const xServerInfo * override;
	void Clear() override;

private:
	std::vector<xServerInfo> ServerList;
};

X_END
