#pragma once
#include "./core_min.hpp"
#include "./core_value_util.hpp"
X_COMMON_BEGIN

template <typename T>
class xOptional final {
	static_assert(!std::is_reference_v<T> && !std::is_const_v<T>);
	using xValueType = std::remove_cv_t<std::remove_reference_t<T>>;

public:
	X_INLINE xOptional()                   = default;
	X_INLINE xOptional(xOptional && Other) = delete;
	X_INLINE ~xOptional() { Steal(_Valid) ? _Holder.Destroy() : Pass(); }

	template <typename... tArgs>
	xOptional(tArgs... Args) {
		_Holder.CreateValue(std::forward<tArgs>(Args)...);
		_Valid = true;
	}

	X_INLINE operator bool() const { return _Valid; }

	X_INLINE xValueType & operator*() {
		assert(_Valid);
		return *_Holder;
	}
	X_INLINE const xValueType & operator*() const {
		assert(_Valid);
		return *_Holder;
	}
	X_INLINE xValueType * operator->() {
		assert(_Valid);
		return &(*_Holder);
	}
	X_INLINE const xValueType * operator->() const { return _Valid ? &(*_Holder) : nullptr; }

	X_INLINE const xValueType & Or(const xValueType & DefaultValue) const { return _Valid ? *_Holder : DefaultValue; }

private:
	bool                _Valid = false;
	xHolder<xValueType> _Holder;
};

X_COMMON_END
