#include "./config.hpp"

#include <cinttypes>

X_BEGIN

void xConfigLoader::Require(std::string & Dst, const char * Key) {
	assert(Key);
	auto Value = Reader->Get(Key);
	if (!Value) {
		X_PFATAL("Failed to get config key: %s", Key);
	}
	Dst = Value;
	return;
}

void xConfigLoader::Require(xNetAddress & Dst, const char * Key) {
	assert(Key);
	auto AddrStr = std::string();
	Require(AddrStr, Key);
	Dst = xNetAddress::Parse(AddrStr);
	if (!Dst) {
		X_PFATAL("Failed to get config key: %s, addr_str=%s", Key, AddrStr.c_str());
	}
	return;
}

void xConfigLoader::Require(int64_t & Dst, const char * Key) {
	assert(Key);
	auto ValueStr = std::string();
	Require(ValueStr, Key);
	Dst = (int64_t)strtoimax(ValueStr.c_str(), nullptr, 10);
	return;
}

void xConfigLoader::Optional(std::string & Dst, const char * Key, const std::string & DefaultValue) {
	assert(Key);
	Dst = Reader->Get(Key, DefaultValue.c_str());
}

void xConfigLoader::Optional(int64_t & Dst, const char * Key, int64_t DefaultValue) {
	assert(Key);
	Dst = Reader->GetInt64(Key, DefaultValue);
}

void xConfigLoader::Optional(bool & Dst, const char * Key, bool DefaultValue) {
	assert(Key);
	Dst = Reader->GetBool(Key, DefaultValue);
}

X_END
