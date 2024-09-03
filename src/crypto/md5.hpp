#pragma once
#include "../core/core_min.hpp"
#include "../core/view.hpp"

X_BEGIN

struct xMd5Result {
	ubyte Digest[16];

	X_INLINE const ubyte * Data() const { return Digest; }
	X_INLINE size_t        Size() const { return sizeof(Digest); }

	X_INLINE xView<const ubyte> View() const { return { Digest, sizeof(Digest) }; }
	X_INLINE                    operator const ubyte *() const { return Digest; }
};

X_API xMd5Result Md5(const void * Source, size_t Size);

X_END
