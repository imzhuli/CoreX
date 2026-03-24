#include "./hmac.hpp"

#include <mbedtls/md.h>

X_BEGIN

xHmacSha256Result HmacSha256(const void * Source, size_t Size, const void * Key, size_t KeySize) {
	xHmacSha256Result R;
	HmacSha256(R, Source, Size, Key, KeySize);
	return R;
}

void HmacSha256(xHmacSha256Result & Output, const void * Source, size_t Size, const void * Key, size_t KeySize) {
	mbedtls_md_hmac(mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), (const unsigned char *)Key, KeySize, (const unsigned char *)Source, Size, (unsigned char *)Output.Data);
}

X_END
