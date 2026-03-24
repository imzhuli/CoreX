#include "./md5.hpp"

#include <mbedtls/md5.h>

X_BEGIN

xMd5Result Md5(const void * Source, size_t Size) {
	xMd5Result R;
	Md5(R, Source, Size);
	return R;
}

void Md5(xMd5Result & Output, const void * Source, size_t Size) {
	mbedtls_md5((const ubyte *)Source, Size, (unsigned char *)Output.Data);
}

X_END
