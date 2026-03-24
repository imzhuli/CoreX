#pragma once
#include "../core/core_min.hpp"
#include "../core/view.hpp"

X_BEGIN

struct xHmacSha256Result {
	ubyte Data[32];
};

X_API xHmacSha256Result HmacSha256(const void * Source, size_t Size, const void * Key, size_t KeySize);
X_API void              HmacSha256(xHmacSha256Result & Output, const void * Source, size_t Size, const void * Key, size_t KeySize);

X_END
