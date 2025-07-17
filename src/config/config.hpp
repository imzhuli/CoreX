#pragma once
#include "../core/core_min.hpp"
#include "../core/ini.hpp"
#include "../core/optional.hpp"
#include "../network/net_address.hpp"

#include <string>

X_BEGIN

class xConfigLoader : xNonCopyable {

public:
	xConfigLoader() = default;
	xConfigLoader(const char * filename) { RenewValue(ReaderOpt, filename); }
	xConfigLoader(const std::string & filename)
		: xConfigLoader(filename.c_str()) {}

	X_INLINE void Reload(const char * filename) { RenewValue(ReaderOpt, filename); }
	X_INLINE void Reload(const std::string & filename) { RenewValue(ReaderOpt, filename.c_str()); }
	X_INLINE      operator bool() const { return ReaderOpt && *ReaderOpt; }

	void Require(std::string & Dst, const char * Key);
	void Require(xNetAddress & Dst, const char * Key);
	void Require(int64_t & Dst, const char * Key);
	void Require(bool & Dst, const char * Key);
	template <typename T>
	std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<int64_t, T> && !std::is_same_v<bool, T>> Require(T & Dst, const char * Key) {
		int64_t Temp;
		Require(Temp, Key);
		Dst = (T)Temp;
	}

	void Optional(std::string & Dst, const char * Key, const std::string & DefaultValue = {});
	void Optional(xNetAddress & Dst, const char * Key, const xNetAddress & DefaultValue = {});
	void Optional(int64_t & Dst, const char * Key, int64_t DefaultValue = 0);
	void Optional(bool & Dst, const char * Key, bool DefaultValue = false);
	template <typename T, typename U>
	std::enable_if_t<std::is_integral_v<T> && !std::is_same_v<int64_t, T> && !std::is_same_v<bool, T>> Optional(T & Dst, const char * Key, U DefaultValue = {}) {
		int64_t Temp;
		Optional(Temp, Key, DefaultValue);
		Dst = (T)Temp;
	}

private:
	xOptional<xIniReader> ReaderOpt;
};

X_END
