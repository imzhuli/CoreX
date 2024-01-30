#include "./sha.hpp"

#include <mbedtls/sha256.h>

X_BEGIN

xSha256Result Sha256(const void * Source, size_t Size) {
	xSha256Result Result;
	mbedtls_sha256((const ubyte *)Source, Size, Result.Digest, 0);
	return Result;
}

X_END
