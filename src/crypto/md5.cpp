#include "./md5.hpp"

#include <mbedtls/md5.h>

X_BEGIN

xMd5Result Md5(const void * Source, size_t Size) {
	xMd5Result Result;
	mbedtls_md5((const ubyte *)Source, Size, Result.Digest);
	return Result;
}

X_END
