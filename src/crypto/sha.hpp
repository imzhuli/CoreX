#pragma once
#include "../core/core_min.hpp"
#include "../core/view.hpp"

X_BEGIN

struct xSha256Result {
	ubyte Digest[32];

	X_INLINE const ubyte * Data() const {
		return Digest;
	}
	X_INLINE size_t Size() const {
		return sizeof(Digest);
	}
	X_INLINE xArrayView<ubyte> View() const {
		return { Digest };
	}
	X_INLINE operator const ubyte *() const {
		return Digest;
	}
};

X_API xSha256Result Sha256(const void * Source, size_t Size);

X_END
