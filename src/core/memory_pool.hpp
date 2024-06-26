#pragma once

#include "./list.hpp"
#include "./memory.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <new>
#include <type_traits>

X_BEGIN

struct xMemoryPoolOptions {
	xAllocator * Allocator         = &DefaultAllocator;
	size_t       InitSize          = 64;
	size_t       Addend            = 128;
	size_t       MultiplierBy100th = 0;
	size_t       MaxSizeIncrement  = 1024;
	size_t       MaxPoolSize       = std::numeric_limits<ssize_t>::max();
};

template <typename T>
class xMemoryPool final {
	static_assert(!std::is_reference_v<T>);

	// !!! important note:
	// here we are using struct (not union), so that if the Constructor throws any exception and is caught outside,
	// the memory pool still remains usable.
	struct xTypeWrapper final {
		alignas(T) ubyte xObjectHolder[sizeof(T)];
		xTypeWrapper * pNext;

		X_INLINE T * Construct() {
			auto pTarget = ((void *)&xObjectHolder);
			new (pTarget) T;
			return static_cast<T *>(pTarget);
		}
		template <typename... CArgs>
		X_INLINE T * ConstructWith(CArgs &&... cargs) {
			auto pTarget = ((void *)&xObjectHolder);
			new (pTarget) T(std::forward<CArgs>(cargs)...);
			return static_cast<T *>(pTarget);
		}
		template <typename... CArgs>
		X_INLINE T * ConstructWithList(CArgs &&... cargs) {
			auto pTarget = ((void *)&xObjectHolder);
			new (pTarget) T{ std::forward<CArgs>(cargs)... };
			return static_cast<T *>(pTarget);
		}

		X_INLINE void Destruct() noexcept {
			reinterpret_cast<T *>(&xObjectHolder)->~T();
		}
	};

	struct xBlock : public xListNode {
		size_t       Count;
		size_t       InitCount = 0;
		xTypeWrapper ResourcePool[1];

		X_INLINE xBlock(size_t count) noexcept
			: Count(count) {
		}
		xBlock(xBlock &&) = delete;
	};

private:
	xAllocator * hAlloc             = nullptr;
	size_t       cInitSize          = 0;
	size_t       cAddend            = 0;
	size_t       cMultiplierBy100th = 0;
	size_t       cMaxSizeIncrement  = 1000;
	size_t       cMaxPoolSize       = std::numeric_limits<ssize_t>::max();

	xList<xBlock>  _BlockList;
	xTypeWrapper * _NextFreeNode = nullptr;
	size_t         _TotalSize    = 0;

public:
	X_INLINE bool Init(const size_t MaxPoolSize, const size_t Addend = 128) {
		assert(MaxPoolSize > 0);
		xMemoryPoolOptions Options = {};
		Options.MaxPoolSize        = (Options.InitSize >= MaxPoolSize) ? Options.InitSize : MaxPoolSize;
		Options.Addend             = Addend;
		return Init(Options);
	}

	X_INLINE bool Init(const xMemoryPoolOptions & Options) {
		assert(Options.Allocator);
		assert(Options.MultiplierBy100th || Options.Addend);
		assert(Options.InitSize >= 1 && Options.MaxSizeIncrement > 0);

		hAlloc             = Options.Allocator;
		cInitSize          = std::min(Options.InitSize, Options.MaxPoolSize);
		cAddend            = Options.Addend;
		cMultiplierBy100th = Options.MultiplierBy100th;
		cMaxSizeIncrement  = Options.MaxSizeIncrement;
		cMaxPoolSize       = Options.MaxPoolSize;

		return AllocBlock(cInitSize) != nullptr;
	}

	X_INLINE void Clean() {
		while (auto NP = _BlockList.PopHead()) {
			hAlloc->Free(NP);
		}
		_NextFreeNode = nullptr;
		_TotalSize    = 0;
	}

	X_INLINE T * Create() {
		if (_NextFreeNode) {
			T * pTarget   = _NextFreeNode->Construct();
			_NextFreeNode = _NextFreeNode->pNext;
			return pTarget;
		}
		auto pLastBlock = _BlockList.Tail();
		if (pLastBlock->Count == pLastBlock->InitCount && !(pLastBlock = ExtendPool())) {
			return nullptr;
		}
		xTypeWrapper * pWrapper = &pLastBlock->ResourcePool[pLastBlock->InitCount];
		T *            pTarget  = pWrapper->Construct();
		++pLastBlock->InitCount;
		return pTarget;
	}

	template <typename... CArgs>
	X_INLINE T * CreateValue(CArgs &&... cargs) {
		if (_NextFreeNode) {
			T * pTarget   = _NextFreeNode->ConstructWith(std::forward<CArgs>(cargs)...);
			_NextFreeNode = _NextFreeNode->pNext;
			return pTarget;
		}
		auto pLastBlock = _BlockList.Tail();
		if (pLastBlock->Count == pLastBlock->InitCount && !(pLastBlock = ExtendPool())) {
			return nullptr;
		}
		xTypeWrapper * pWrapper = &pLastBlock->ResourcePool[pLastBlock->InitCount];
		T *            pTarget  = pWrapper->ConstructWith(std::forward<CArgs>(cargs)...);
		++pLastBlock->InitCount;
		return pTarget;
	}

	template <typename... CArgs>
	X_INLINE T * CreateValueWithList(CArgs &&... cargs) {
		if (_NextFreeNode) {
			T * pTarget   = _NextFreeNode->ConstructWithList(std::forward<CArgs>(cargs)...);
			_NextFreeNode = _NextFreeNode->pNext;
			return pTarget;
		}
		auto pLastBlock = _BlockList.Tail();
		if (pLastBlock->Count == pLastBlock->InitCount && !(pLastBlock = ExtendPool())) {
			return nullptr;
		}
		xTypeWrapper * pWrapper = &pLastBlock->ResourcePool[pLastBlock->InitCount];
		T *            pTarget  = pWrapper->ConstructWithList(std::forward<CArgs>(cargs)...);
		++pLastBlock->InitCount;
		return pTarget;
	}

	X_INLINE void Destroy(T * pTarget) {
		assert(pTarget);
		xTypeWrapper * pWrapper = reinterpret_cast<xTypeWrapper *>(reinterpret_cast<ubyte *>(pTarget) - offsetof(xTypeWrapper, xObjectHolder));
		pWrapper->Destruct();
		pWrapper->pNext = _NextFreeNode;
		_NextFreeNode   = pWrapper;
	}

private:
	X_INLINE xBlock * ExtendPool() {
		size_t maxAddSize = std::min(cMaxSizeIncrement, cMaxPoolSize - _TotalSize);
		if (!maxAddSize) {
			return nullptr;
		}
		size_t addSize = std::min(_TotalSize * cMultiplierBy100th / 100 + cAddend, maxAddSize);
		return AllocBlock(addSize);
	}

	xBlock * AllocBlock(size_t count) {
		assert(count);
		size_t   totalSize = (sizeof(xBlock) - sizeof(xTypeWrapper)) + sizeof(xTypeWrapper) * count;
		xBlock * pBlock    = (xBlock *)hAlloc->Alloc(totalSize, AllocAlignSize<xBlock>);
		if (!pBlock) {
			return nullptr;
		}
		new ((void *)pBlock) xBlock(count);
		// Init queue:
		auto & block = *pBlock;
		_BlockList.AddTail(block);
		_TotalSize += count;
		return pBlock;
	}
};

X_END
