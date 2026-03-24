#pragma once
#include "../core/core_min.hpp"

X_BEGIN

struct xMd5Result {
	ubyte Data[16];
};

X_API xMd5Result Md5(const void * Source, size_t Size);
X_API void       Md5(xMd5Result & Output, const void * Source, size_t Size);

X_END
