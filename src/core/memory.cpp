#include "./memory.hpp"

X_BEGIN

void * xAllocator::Alloc(size_t vxSize, size_t vxAlignment) noexcept {
	return XelAlignedAlloc(vxSize, vxAlignment);
}

void xAllocator::Free(void * vpObject) noexcept {
	XelAlignedFree(vpObject);
}

X_END
