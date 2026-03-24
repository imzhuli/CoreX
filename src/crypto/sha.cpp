#include "./sha.hpp"

#include <mbedtls/sha256.h>

X_BEGIN

xSha256Result Sha256(const void * Source, size_t Size) {
	xSha256Result R;
	Sha256(R, Source, Size);
	return R;
}

void Sha256(xSha256Result & Output, const void * Source, size_t Size) {
	mbedtls_sha256((const ubyte *)Source, Size, (unsigned char *)Output.Data, 0);
}

X_END
