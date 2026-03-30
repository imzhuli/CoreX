#pragma once

#include "./core_value_util.hpp"
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
	static constexpr const size_t DefaultInitSize    = 4096;
	static constexpr const size_t DefaultAddend      = 4096;
	static constexpr const size_t DefaultMaxPoolSize = std::numeric_limits<ssize_t>::max();

	xAllocator * Allocator   = nullptr;
	size_t       InitSize    = DefaultInitSize;
	size_t       Addend      = DefaultAddend;
	size_t       MaxPoolSize = DefaultMaxPoolSize;
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

		X_INLINE T * Construct() noexcept {
			return ConstructAt<T>((void *)&xObjectHolder);
		}
		template <typename... CArgs>
		X_INLINE T * ConstructWith(CArgs &&... cargs) noexcept {
			return ConstructAtWith<T>((void *)&xObjectHolder, std::forward<CArgs>(cargs)...);
		}
		template <typename... CArgs>
		X_INLINE T * ConstructWithList(CArgs &&... cargs) noexcept {
			return ConstructAtWithList<T>((void *)&xObjectHolder, std::forward<CArgs>(cargs)...);
		}
		X_INLINE void Destruct() noexcept {
			DestructAt<T>((void *)&xObjectHolder);
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
	xAllocator * hAlloc       = nullptr;
	size_t       cInitSize    = 0;
	size_t       cAddend      = 0;
	size_t       cMaxPoolSize = std::numeric_limits<ssize_t>::max();

	xList<xBlock>  _BlockList;
	xTypeWrapper * _NextFreeNode = nullptr;
	size_t         _TotalSize    = 0;

	// internal allocator, to avoid init sequence issue.
	xAllocator _DefaultAllocator;

public:
	X_INLINE bool Init(const xMemoryPoolOptions & Options) {
		assert(Options.InitSize == Options.MaxPoolSize || Options.Addend);
		assert(Options.InitSize >= 1);

		hAlloc       = Options.Allocator ? Options.Allocator : &_DefaultAllocator;
		cInitSize    = std::min(Options.InitSize, Options.MaxPoolSize);
		cAddend      = Options.Addend;
		cMaxPoolSize = Options.MaxPoolSize;

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
			auto pTarget  = _NextFreeNode->Construct();
			_NextFreeNode = _NextFreeNode->pNext;
			return pTarget;
		}
		auto pLastBlock = _BlockList.Tail();
		if (pLastBlock->Count == pLastBlock->InitCount && !(pLastBlock = ExtendPool())) {
			return nullptr;
		}
		xTypeWrapper * pWrapper = &pLastBlock->ResourcePool[pLastBlock->InitCount];
		auto           pTarget  = pWrapper->Construct();
		++pLastBlock->InitCount;
		return pTarget;
	}

	template <typename... CArgs>
	X_INLINE T * CreateValue(CArgs &&... cargs) {
		if (_NextFreeNode) {
			auto pTarget  = _NextFreeNode->ConstructWith(std::forward<CArgs>(cargs)...);
			_NextFreeNode = _NextFreeNode->pNext;
			return pTarget;
		}
		auto pLastBlock = _BlockList.Tail();
		if (pLastBlock->Count == pLastBlock->InitCount && !(pLastBlock = ExtendPool())) {
			return nullptr;
		}
		xTypeWrapper * pWrapper = &pLastBlock->ResourcePool[pLastBlock->InitCount];
		auto           pTarget  = pWrapper->ConstructWith(std::forward<CArgs>(cargs)...);
		++pLastBlock->InitCount;
		return pTarget;
	}

	template <typename... CArgs>
	X_INLINE T * CreateValueWithList(CArgs &&... cargs) {
		if (_NextFreeNode) {
			auto pTarget  = _NextFreeNode->ConstructWithList(std::forward<CArgs>(cargs)...);
			_NextFreeNode = _NextFreeNode->pNext;
			return pTarget;
		}
		auto pLastBlock = _BlockList.Tail();
		if (pLastBlock->Count == pLastBlock->InitCount && !(pLastBlock = ExtendPool())) {
			return nullptr;
		}
		xTypeWrapper * pWrapper = &pLastBlock->ResourcePool[pLastBlock->InitCount];
		auto           pTarget  = pWrapper->ConstructWithList(std::forward<CArgs>(cargs)...);
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
		size_t addSize = std::min(cAddend, cMaxPoolSize - _TotalSize);
		if (!addSize) {
			return nullptr;
		}
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
