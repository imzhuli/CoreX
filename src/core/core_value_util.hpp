#pragma once
#include "./core_min.hpp"

X_COMMON_BEGIN

template <size_t TargetSize, size_t Alignment = alignof(std::max_align_t)>
class xDummy final : xNonCopyable {
public:
	template <typename T>
	X_INLINE void CreateAs() {
		static_assert(Alignment >= alignof(T));
		static_assert(sizeof(_PlaceHolder) >= sizeof(T));
		new ((void *)_PlaceHolder) T;
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

private:
	alignas(Alignment) ubyte _PlaceHolder[TargetSize];
};

template <typename T>
class xHolder final : xNonCopyable {
public:
	X_INLINE xHolder() = default;
	X_INLINE ~xHolder() = default;

	X_INLINE void Create() { new ((void *)_PlaceHolder) T; }

	template <typename... tArgs>
	X_INLINE T * CreateValue(tArgs &&... Args) {
		auto ObjectPtr = new ((void *)_PlaceHolder) T(std::forward<tArgs>(Args)...);
		return ObjectPtr;
	}

	template <typename... tArgs>
	X_INLINE T * CreateValueWithList(tArgs &&... Args) {
		auto ObjectPtr = new ((void *)_PlaceHolder) T{ std::forward<tArgs>(Args)... };
		return ObjectPtr;
	}

	X_INLINE void Destroy() { Get()->~T(); }

	X_INLINE T *	   operator->() { return Get(); }
	X_INLINE const T * operator->() const { return Get(); }

	X_INLINE T &	   operator*() { return *Get(); }
	X_INLINE const T & operator*() const { return *Get(); }

	X_INLINE T *	   Get() { return reinterpret_cast<T *>(_PlaceHolder); }
	X_INLINE const T * Get() const { return reinterpret_cast<const T *>(_PlaceHolder); }

private:
	alignas(T) ubyte _PlaceHolder[sizeof(T)];
};

X_COMMON_END
