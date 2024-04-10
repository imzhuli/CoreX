#pragma once
#include "./core_min.hpp"

#include <atomic>
#include <functional>
#include <memory>

X_BEGIN

template <bool cAtomic = false, typename T = int64_t>
class xRetainableBase;

// ref-counter
template <bool cAtomic, typename T>
class xRetainableBase {
	static_assert(std::is_integral_v<T> && std::is_signed_v<T>);

public:
	using RealType = std::conditional_t<cAtomic, std::atomic<T>, T>;

protected:
	X_INLINE xRetainableBase() = default;
	X_INLINE xRetainableBase(T initCount)
		: _Count(initCount) {
	}
	X_INLINE                   xRetainableBase(const xRetainableBase &) {};
	X_INLINE xRetainableBase & operator=(const xRetainableBase &) {
		return *this;
	}

public:
	X_INLINE void Retain(T increment = 1) const {
		_Count += increment;
	}
	X_INLINE T Release(T decrement = 1) const {
		return _Count -= decrement;
	}
	X_INLINE T GetRetainCount() const {
		return static_cast<T>(_Count);
	}
	X_INLINE void SetRetainCount(T resetCount) const {
		_Count = resetCount;
	}

private:
	mutable RealType _Count{ 1 };
};

using xRetainable8  = xRetainableBase<false, int8_t>;
using xRetainable16 = xRetainableBase<false, int16_t>;
using xRetainable32 = xRetainableBase<false, int32_t>;
using xRetainable64 = xRetainableBase<false, int64_t>;
using xRetainable   = xRetainable64;

using xRetainableAtomic8  = xRetainableBase<true, int8_t>;
using xRetainableAtomic16 = xRetainableBase<true, int16_t>;
using xRetainableAtomic32 = xRetainableBase<true, int32_t>;
using xRetainableAtomic64 = xRetainableBase<true, int64_t>;
using xRetainableAtomic   = xRetainableAtomic64;

namespace __detail__ {
	template <typename T>
	struct __AllocAlignSize__ {
#if defined(__APPLE__)
		static constexpr const size_t value = alignof(T) >= sizeof(void *) ? alignof(T) : sizeof(void *);
#else
		static constexpr const size_t value = alignof(T);
#endif
	};
}  // namespace __detail__

template <typename T>
inline constexpr size_t AllocAlignSize = __detail__::__AllocAlignSize__<T>::value;

class xAllocator {
public:
	/**
	 * @brief Allocat memory,
	 * @param vxSize must be multiple of
	 * @param vxAlignment alignment
	 * @return address of Allocaed memroy
	 * @exception bad_Alloc
	 */
	X_API_MEMBER virtual void * Alloc(size_t vxSize, size_t vxAlignment) noexcept;
	/**
	 * @brief Free Allocated memory, null pointer should be accepted
	 */
	X_API_MEMBER virtual void Free(const void * vpObject) noexcept;

	template <typename T>
	X_INLINE T * Create() {
		void * p = this->Alloc(sizeof(T), AllocAlignSize<T>);
		try {
			new (p) T;
		} catch (...) {
			this->Free(p);
			throw;
		}
		return (T *)p;
	}

	template <typename T, typename... Args>
	X_INLINE T * CreateValue(Args &&... args) {
		void * p = this->Alloc(sizeof(T), AllocAlignSize<T>);
		try {
			new (p) T(std::forward<Args>(args)...);
		} catch (...) {
			this->Free(p);
			throw;
		}
		return (T *)p;
	}

	template <typename T, typename... Args>
	X_INLINE T * CreateValueWithList(Args &&... args) {
		void * p = this->Alloc(sizeof(T), AllocAlignSize<T>);
		try {
			new (p) T{ std::forward<Args>(args)... };
		} catch (...) {
			this->Free(p);
			throw;
		}
		return (T *)p;
	}

	template <typename T>
	X_INLINE T * AlignedCreate(size_t vxAlignment) {
		void * p = this->Alloc(sizeof(T), vxAlignment);
		try {
			new (p) T;
		} catch (...) {
			this->Free(p);
			throw;
		}
		return (T *)p;
	}

	template <typename T, typename... Args>
	X_INLINE T * AlignedCreateValue(size_t vxAlignment, Args &&... args) {
		void * p = this->Alloc(sizeof(T), vxAlignment);
		try {
			new (p) T(std::forward<Args>(args)...);
		} catch (...) {
			this->Free(p);
			throw;
		}
		return (T *)p;
	}

	template <typename T, typename... Args>
	X_INLINE T * AlignedCreateValueWithList(size_t vxAlignment, Args &&... args) {
		void * p = this->Alloc(sizeof(T), vxAlignment);
		try {
			new (p) T{ std::forward<Args>(args)... };
		} catch (...) {
			this->Free(p);
			throw;
		}
		return (T *)p;
	}

	template <typename T>
	X_INLINE T * CreateArray(size_t n) {
		void * p = this->Alloc(sizeof(T) * n, AllocAlignSize<T>);
		if constexpr (!std::is_trivially_constructible_v<T>) {
			try {
				new (p) T[n];
			} catch (...) {
				this->Free(p);
				throw;
			}
		}
		return (T *)p;
	}

	template <typename T, typename... Args>
	X_INLINE T * CreateValueArray(size_t n, Args &&... args) {
		void * p = this->Alloc(sizeof(T) * n, AllocAlignSize<T>);
		try {
			new (p) T[n]{ std::forward<Args>(args)... };
		} catch (...) {
			this->Free(p);
			throw;
		}
		return (T *)p;
	}

	template <typename T>
	X_INLINE void Destroy(T * pObject) {
		pObject->~T();
		this->Free(pObject);
	}

	template <typename T>
	X_INLINE void DestroyArray(T * pStart, size_t n) {
		if constexpr (!std::is_trivially_destructible_v<T>) {
			T * pObject = pStart;
			for (size_t i = 0; i < n; ++i) {
				pObject->~T();
				++pObject;
			}
		}
		this->Free(pStart);
	}

public:
	xAllocator()              = default;
	xAllocator(xAllocator &&) = delete;
	virtual ~xAllocator();
};

X_API xAllocator DefaultAllocator;

X_END
