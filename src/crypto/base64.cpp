#include "./base64.hpp"

#include <mbedtls/base64.h>

X_BEGIN

std::string Base64Encode(const void * ToEncode, size_t Size) {
	auto Result = std::string();
	Result.resize(Size * 2);
	auto OLen = size_t(0);
	mbedtls_base64_encode((ubyte *)Result.data(), Result.size(), &OLen, (const ubyte *)ToEncode, Size);
	Result.resize(OLen);
	return Result;
}

std::string Base64Decode(const void * ToDecode, size_t Size) {
	auto Result = std::string();
	Result.resize(Size);
	auto OLen = size_t(0);
	mbedtls_base64_decode((ubyte *)Result.data(), Result.size(), &OLen, (const ubyte *)ToDecode, Size);
	Result.resize(OLen);
	return Result;
}

X_END
