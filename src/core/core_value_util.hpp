// clang-format off
#pragma once

#include "./core_min.hpp"

X_COMMON_BEGIN

template <typename T>  
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

	X_STATIC_INLINE xHolder * O2H(T* ObjectPtr) {
		return X_Entry(ObjectPtr, xHolder, _PlaceHolder);
	}
	X_STATIC_INLINE const xHolder * O2H(const T* ObjectPtr) {
		return X_Entry(ObjectPtr, const xHolder, _PlaceHolder);
	}

private:
	alignas(T) ubyte _PlaceHolder[sizeof(T)];
};

X_COMMON_END
