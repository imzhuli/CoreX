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
	xAllocator * Allocator         = nullptr;
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
			try {
				new (pTarget) T;
			} catch (...) {
				return nullptr;
			}
			return static_cast<T *>(pTarget);
		}
		template <typename... CArgs>
		X_INLINE T * ConstructWith(CArgs &&... cargs) {
			auto pTarget = ((void *)&xObjectHolder);
			try {
				new (pTarget) T(std::forward<CArgs>(cargs)...);
			} catch (...) {
				return nullptr;
			}
			return static_cast<T *>(pTarget);
		}
		template <typename... CArgs>
		X_INLINE T * ConstructWithList(CArgs &&... cargs) {
			auto pTarget = ((void *)&xObjectHolder);
			try {
				new (pTarget) T{ std::forward<CArgs>(cargs)... };
			} catch (...) {
				return nullptr;
			}
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

	// internal allocator, to avoid init sequence issue.
	xAllocator _DefaultAllocator;

public:
	X_INLINE bool Init(const size_t MaxPoolSize, const size_t Addend = 128) {
		assert(MaxPoolSize > 0);
		xMemoryPoolOptions Options = {};
		Options.MaxPoolSize        = (Options.InitSize >= MaxPoolSize) ? Options.InitSize : MaxPoolSize;
		Options.Addend             = Addend;
		return Init(Options);
	}

	X_INLINE bool Init(const xMemoryPoolOptions & Options) {
		assert(Options.InitSize == Options.MaxPoolSize || Options.MultiplierBy100th || Options.Addend);
		assert(Options.InitSize >= 1);

		hAlloc             = Options.Allocator ? Options.Allocator : &_DefaultAllocator;
		cInitSize          = std::min(Options.InitSize, Options.MaxPoolSize);
		cAddend            = Options.Addend;
		cMultiplierBy100th = Options.MultiplierBy100th;
		cMaxSizeIncrement  = Options.MaxSizeIncrement ? Options.MaxSizeIncrement : std::min(Options.InitSize, Options.Addend);
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
			if (auto pTarget = _NextFreeNode->Construct()) {
				_NextFreeNode = _NextFreeNode->pNext;
				return pTarget;
			}
			return nullptr;
		}
		auto pLastBlock = _BlockList.Tail();
		if (pLastBlock->Count == pLastBlock->InitCount && !(pLastBlock = ExtendPool())) {
			return nullptr;
		}
		xTypeWrapper * pWrapper = &pLastBlock->ResourcePool[pLastBlock->InitCount];
		if (auto pTarget = pWrapper->Construct()) {
			++pLastBlock->InitCount;
			return pTarget;
		}
		return nullptr;
	}

	template <typename... CArgs>
	X_INLINE T * CreateValue(CArgs &&... cargs) {
		if (_NextFreeNode) {
			if (auto pTarget = _NextFreeNode->ConstructWith(std::forward<CArgs>(cargs)...)) {
				_NextFreeNode = _NextFreeNode->pNext;
				return pTarget;
			}
			return nullptr;
		}
		auto pLastBlock = _BlockList.Tail();
		if (pLastBlock->Count == pLastBlock->InitCount && !(pLastBlock = ExtendPool())) {
			return nullptr;
		}
		xTypeWrapper * pWrapper = &pLastBlock->ResourcePool[pLastBlock->InitCount];
		if (auto pTarget = pWrapper->ConstructWith(std::forward<CArgs>(cargs)...)) {
			++pLastBlock->InitCount;
			return pTarget;
		}
		return nullptr;
	}

	template <typename... CArgs>
	X_INLINE T * CreateValueWithList(CArgs &&... cargs) {
		if (_NextFreeNode) {
			if (auto pTarget = _NextFreeNode->ConstructWithList(std::forward<CArgs>(cargs)...)) {
				_NextFreeNode = _NextFreeNode->pNext;
				return pTarget;
			}
			return nullptr;
		}
		auto pLastBlock = _BlockList.Tail();
		if (pLastBlock->Count == pLastBlock->InitCount && !(pLastBlock = ExtendPool())) {
			return nullptr;
		}
		xTypeWrapper * pWrapper = &pLastBlock->ResourcePool[pLastBlock->InitCount];
		if (auto pTarget = pWrapper->ConstructWithList(std::forward<CArgs>(cargs)...)) {
			++pLastBlock->InitCount;
			return pTarget;
		}
		return nullptr;
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

namespace __memory_pool__ {
	class xFixedObjectPoolBase : xNonCopyable {
	protected:
		union xPaddingNode {
			xPaddingNode * NextFreeNode;  // before alloc
			bool           IsPooled;      // after alloc
		};

	protected:
		X_API_MEMBER bool   CreateNodePool(size_t NodeSize, size_t PoolSize);
		X_API_MEMBER void   DestroyNodePool();
		X_API_MEMBER void * Alloc();
		X_API_MEMBER void   Free(void * P);

	private:
		size_t          NodeSize;
		size_t          PoolSize;
		size_t          InitedSize;
		xPaddingNode *  NextFreeNode;
		unsigned char * Pool = nullptr;
	};
}  // namespace __memory_pool__

template <typename T>
class xFixedObjectPool final : __memory_pool__::xFixedObjectPoolBase {
private:
	static_assert(!std::is_const_v<T>);
	static_assert(!std::is_reference_v<T>);
	struct xNode {
		xPaddingNode Header;
		T            Object;
	};

public:
	X_INLINE bool Init(size_t PoolSize) {
		return CreateNodePool(sizeof(xNode), PoolSize);
	}
	X_INLINE void Clean() {
		DestroyNodePool();
	}

	X_INLINE T * Create() {
		auto NP = static_cast<xNode *>(Alloc());
		if (!NP) {
			return nullptr;
		}
		try {
			new ((void *)&NP->Object) T;
		} catch (...) {
			Free(NP);
			return nullptr;
		}
		return &NP->Object;
	}

	template <typename... tArgs>
	X_INLINE T * CreateValue(tArgs &&... Args) {
		auto NP = static_cast<xNode *>(Alloc());
		if (!NP) {
			return nullptr;
		}
		try {
			new ((void *)&NP->Object) T(std::forward<tArgs>(Args)...);
		} catch (...) {
			Free(NP);
			return nullptr;
		}
		return &NP->Object;
	}

	template <typename... tArgs>
	X_INLINE T * CreateWithList(tArgs &&... Args) {
		auto NP = static_cast<xNode *>(Alloc());
		if (!NP) {
			return nullptr;
		}
		try {
			new ((void *)&NP->Object) T{ std::forward<tArgs>(Args)... };
		} catch (...) {
			Free(NP);
			return nullptr;
		}
		return &NP->Object;
	}

	X_INLINE void Destroy(T * OP) {
		Free(X_Entry(OP, xNode, Object));
	}
};

X_END
