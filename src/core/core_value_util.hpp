#pragma once
#include "./core_min.hpp"

X_COMMON_BEGIN

template <typename T>
class xHolder final : xNonCopyable {
public:
	X_INLINE xHolder()  = default;
	X_INLINE ~xHolder() = default;

	X_INLINE void Create() { new ((void *)_PlaceHolder) T; }
	template <typename... tArgs>
	X_INLINE void CreateValue(tArgs &&... Args) { new ((void *)_PlaceHolder) T(std::forward<tArgs>(Args)...); }
	template <typename... tArgs>
	X_INLINE void CreateValueWithList(tArgs &&... Args) { new ((void *)_PlaceHolder) T{ std::forward<tArgs>(Args)... }; }
	X_INLINE void Destroy() { GetAddress()->~T(); }

	X_INLINE T *       operator->() { return GetAddress(); }
	X_INLINE const T * operator->() const { return GetAddress(); }

	X_INLINE T &       operator*() { return *GetAddress(); }
	X_INLINE const T & operator*() const { return *GetAddress(); }

	X_INLINE T *       GetAddress() { return reinterpret_cast<T *>(_PlaceHolder); }
	X_INLINE const T * GetAddress() const { return reinterpret_cast<const T *>(_PlaceHolder); }

	X_STATIC_INLINE xHolder * GetHolder(T * ObjectPtr) {
		return X_Entry(ObjectPtr, xHolder, _PlaceHolder);
	}
	X_STATIC_INLINE const xHolder * GetHolder(const T * ObjectPtr) {
		return X_Entry(ObjectPtr, xHolder, _PlaceHolder);
	}

private:
	alignas(T) ubyte _PlaceHolder[sizeof(T)];
};

template <typename T>
class xAutoHolder final : xNonCopyable {
public:
	X_INLINE xAutoHolder() {
		_Holder.Create();
	}
	template <typename... tArgs>
	X_INLINE xAutoHolder(tArgs &&... Args) {
		_Holder.CreateValue(std::forward<tArgs>(Args)...);
	}
	template <typename U>
	X_INLINE xAutoHolder(std::initializer_list<U> Init) {
		_Holder.CreateValueWithList(Init);
	}
	X_INLINE ~xAutoHolder() {
		_Holder.Destroy();
	}

	X_INLINE void Reset() {
		_Holder.Destroy();
		_Holder.Create();
	}
	template <typename... tArgs>
	X_INLINE void ResetValue(tArgs &&... Args) {
		_Holder.Destroy();
		_Holder.CreateValue(std::forward<tArgs>(Args)...);
	}
	template <typename... tArgs>
	X_INLINE void ResetValueWithList(tArgs &&... Args) {
		_Holder.Destroy();
		_Holder.CreateValueWithList(std::forward<tArgs>(Args)...);
	}

	X_INLINE T *       operator->() { return _Holder.GetAddress(); }
	X_INLINE const T * operator->() const { return _Holder.GetAddress(); }

	X_INLINE T &       operator*() { return *_Holder.GetAddress(); }
	X_INLINE const T & operator*() const { return *_Holder.GetAddress(); }

private:
	xHolder<T> _Holder;
};

X_COMMON_END
