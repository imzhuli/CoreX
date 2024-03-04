#pragma once
#include "./core_value_util.hpp"

X_BEGIN

class xVersionNumber {
public:
	using xNative = int64_t;

public:
	static constexpr const xNative DISABLE_MASK  = 0x8000'0000'0000'0000;
	static constexpr const xNative INVALID_VALUE = -1;
	static_assert(INVALID_VALUE & DISABLE_MASK);

	X_INLINE xVersionNumber() = default;
	X_INLINE xVersionNumber(xNative NewValue)
		: Native(NewValue) {
	}
	X_INLINE xVersionNumber(const xVersionNumber &) = default;

	X_INLINE operator xNative() const {
		return Native;
	}
	X_INLINE bool IsValid() const {
		return !(Native & DISABLE_MASK);
	}
	X_INLINE bool IsEqualTo(const xVersionNumber & Other) const {
		return IsValid() ? (Native == Other.Native) : false;
	}

private:
	xNative Native = INVALID_VALUE;
};

template <typename T>
class xVersion final : xNonCopyable {
private:
	using RawType = std::remove_cvref_t<T>;
	static_assert(!std::is_array_v<RawType>);
	static_assert(std::is_copy_constructible_v<RawType> || std::is_move_constructible_v<RawType>);

public:
	X_INLINE xVersion() = default;
	X_INLINE ~xVersion() {
		TestAndDisable();
	}

	X_INLINE const RawType & Get() const {
		assert(IsEnabled());
		return *ValueHolder;
	}

	X_INLINE void Enable() {
		assert(!IsEnabled());
		ValueHolder.Create();
		EnableVersion();
	}

	template <typename... tArgs>
	X_INLINE void EnableValue(tArgs &&... Args) {
		assert(!IsEnabled());
		ValueHolder.CreateValue(std::forward<tArgs>(Args)...);
		EnableVersion();
	}

	X_INLINE void Disable() {
		assert(IsEnabled());
		DisableVersion();
		ValueHolder.Destroy();
	}

	X_INLINE void TestAndDisable() {
		if (!IsEnabled()) {
			return;
		}
		Disable();
	}

	X_INLINE bool IsEnabled() const {
		return xVersionNumber(Version).IsValid();
	}

	X_INLINE xVersionNumber GetVersion() const {
		return IsEnabled() ? xVersionNumber(Version) : xVersionNumber();
	}

	X_INLINE bool IsVersion(int64_t V) const {
		return IsEnabled() && V == Version;
	}

private:
	X_INLINE void EnableVersion() {
		++Version;
		Version &= ~xVersionNumber::DISABLE_MASK;
	}
	X_INLINE void DisableVersion() {
		Version |= xVersionNumber::DISABLE_MASK;
	}

private:
	xHolder<RawType>        ValueHolder = {};
	xVersionNumber::xNative Version     = xVersionNumber::INVALID_VALUE;
};

template <>
class xVersion<void> {};

X_END
