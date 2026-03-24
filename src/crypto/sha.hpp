#pragma once
#include "../core/core_min.hpp"

X_BEGIN

struct xSha256Result {
	byte Data[32];
};

X_API xSha256Result Sha256(const void * Source, size_t Size);
X_API void          Sha256(xSha256Result & Output, const void * Source, size_t Size);

X_END
