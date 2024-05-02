#include "./memory.hpp"

X_BEGIN

xAllocator DefaultAllocator;

void * xAllocator::Alloc(size_t vxSize, size_t vxAlignment) noexcept {
	return XelAlignedAlloc(vxSize, vxAlignment);
}

void xAllocator::Free(const void * vpObject) noexcept {
	XelAlignedFree((void *)vpObject);
}

X_END
