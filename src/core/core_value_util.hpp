#pragma once
#include "./core_min.hpp"

X_COMMON_BEGIN

template <size_t TargetSize, size_t Alignment = alignof(std::max_align_t)>
class xDummy final : xNonCopyable {
public:
	template <typename T>
	X_INLINE T & CreateAs() {
		static_assert(Alignment >= alignof(T));
		static_assert(sizeof(_PlaceHolder) >= sizeof(T));
		return *new ((void *)_PlaceHolder) T;
	}

	template <typename T, typename... tArgs>
	X_INLINE T & CreateValueAs(tArgs &&... Args) {
		static_assert(Alignment >= alignof(T));
		static_assert(sizeof(_PlaceHolder) >= sizeof(T));
		return *(new ((void *)_PlaceHolder) T(std::forward<tArgs>(Args)...));
	}

	template <typename T, typename... tArgs>
	X_INLINE T & CreateValueWithListAs(tArgs &&... Args) {
		static_assert(Alignment >= alignof(T));
		static_assert(sizeof(_PlaceHolder) >= sizeof(T));
		return *(new ((void *)_PlaceHolder) T{ std::forward<tArgs>(Args)... });
	}

	template <typename T>
	X_INLINE void DestroyAs() {
		static_assert(Alignment >= alignof(T));
		static_assert(sizeof(_PlaceHolder) >= sizeof(T));
		reinterpret_cast<T *>(_PlaceHolder)->~T();
	}

	template <typename T>
	X_INLINE T & As() {
		static_assert(Alignment >= alignof(T));
		static_assert(sizeof(_PlaceHolder) >= sizeof(T));
		return reinterpret_cast<T &>(_PlaceHolder);
	}

	template <typename T>
	X_INLINE const T & As() const {
		static_assert(Alignment >= alignof(T));
		static_assert(sizeof(_PlaceHolder) >= sizeof(T));
		return reinterpret_cast<const T &>(_PlaceHolder);
	}

	static constexpr const size_t Size = TargetSize;

	template <typename T>
	X_INLINE static std::conditional_t<std::is_const_v<T>, const xDummy *, xDummy *> CastPtr(T * RealObjectPtr) {
		static_assert(Alignment >= alignof(T));
		static_assert(sizeof(_PlaceHolder) >= sizeof(T));
		static_assert(offsetof(xDummy, _PlaceHolder) == 0);
		return X_Entry(AddressOf(*RealObjectPtr), xDummy, _PlaceHolder);
	}

	template <typename T>
	X_INLINE static std::conditional_t<std::is_const_v<T>, const xDummy &, xDummy &> CastRef(T & RealObject) {
		static_assert(Alignment >= alignof(T));
		static_assert(sizeof(_PlaceHolder) >= sizeof(T));
		static_assert(offsetof(xDummy, _PlaceHolder) == 0);
		return *X_Entry(AddressOf(RealObject), xDummy, _PlaceHolder);
	}

private:
	using xPlaceHolder = ubyte[TargetSize];
	alignas(Alignment) xPlaceHolder _PlaceHolder;
};

template <typename T>  // clang-format off
class xHolder final : xNonCopyable {
public:
	X_INLINE xHolder() = default;
	X_INLINE ~xHolder() = default;

	X_INLINE void Create() { new ((void *)_PlaceHolder) T; }
	template <typename... tArgs>
	X_INLINE void CreateValue(tArgs &&... Args) { new ((void *)_PlaceHolder) T(std::forward<tArgs>(Args)...); }
	template <typename... tArgs>
	X_INLINE void CreateValueWithList(tArgs &&... Args) { new ((void *)_PlaceHolder) T{ std::forward<tArgs>(Args)... }; }

	X_INLINE void Destroy() { GetAddress()->~T(); }

	X_INLINE T *	   operator->() { return GetAddress(); }
	X_INLINE const T * operator->() const { return GetAddress(); }

	X_INLINE T &	   operator*() { return *GetAddress(); }
	X_INLINE const T & operator*() const { return *GetAddress(); }

	X_INLINE T *	   GetAddress() { return reinterpret_cast<T *>(_PlaceHolder); }
	X_INLINE const T * GetAddress() const { return reinterpret_cast<const T *>(_PlaceHolder); }

	X_STATIC_INLINE xHolder * O2H(T* Object) {
		auto PP = reinterpret_cast<ubyte*>(Object);
		return X_Entry(PP, xHolder, _PlaceHolder);
	}
	X_STATIC_INLINE const xHolder * O2H(const T* Object) {
		return O2H(const_cast<T*>(Object));
	}

private:
	alignas(T) ubyte _PlaceHolder[sizeof(T)];
};

X_COMMON_END
