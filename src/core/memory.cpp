#include "./memory.hpp"

X_BEGIN

void * xAllocator::Alloc(size_t vxSize, size_t vxAlignment) noexcept {
	return XelAlignedAlloc(vxSize, vxAlignment);
}

void xAllocator::Free(void * vpObject) noexcept {
	XelAlignedFree(vpObject);
}

static_assert(sizeof(xRetainable) == 4);
static_assert(sizeof(xRetainableAtomic) == 4);

X_END
