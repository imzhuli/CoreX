#pragma once
#include "./core_min.hpp"
X_COMMON_BEGIN

template <typename T>
class xOptional final {
	static_assert(!std::is_reference_v<T> && !std::is_const_v<T>);
	using Type       = std::remove_cv_t<std::remove_reference_t<T>>;
	using xCaster    = xRefCaster<T>;
	using xValueType = typename xCaster::Type;

public:
	X_INLINE xOptional()                   = default;
	X_INLINE xOptional(xOptional && Other) = delete;
	X_INLINE ~xOptional() { Steal(_Valid) ? Destroy() : Pass(); }

	template <typename... tArgs>
	X_INLINE xOptional(tArgs &&... Args) {
		new ((void *)_Holder) Type(std::forward<tArgs>(Args)...);
		_Valid = true;
	}

	X_INLINE        operator bool() const { return _Valid; }
	X_INLINE auto & operator*() {
		assert(_Valid);
		return GetValueReference();
	}
	X_INLINE auto & operator*() const {
		assert(_Valid);
		return GetValueReference();
	}

	X_INLINE auto * operator->() { return _Valid ? &GetValueReference() : nullptr; }
	X_INLINE auto * operator->() const { return _Valid ? &GetValueReference() : nullptr; }

	X_INLINE const xValueType & Or(const xValueType & DefaultValue) const { return _Valid ? GetValueReference() : DefaultValue; }

private:
	X_INLINE Type &       GetReference() { return reinterpret_cast<Type &>(_Holder); }
	X_INLINE const Type & GetReference() const { return reinterpret_cast<const Type &>(_Holder); }
	X_INLINE auto &       GetValueReference() { return xCaster::Get(GetReference()); }
	X_INLINE auto &       GetValueReference() const { return xCaster::Get(GetReference()); }
	X_INLINE void         Destroy() { GetReference().~Type(); }

private:
	bool _Valid = false;
	alignas(Type) ubyte _Holder[sizeof(Type)];
};

template <typename U>
xOptional(const U & Value) -> xOptional<U>;

template <typename U>
xOptional(U && Value) -> xOptional<U>;

X_COMMON_END
