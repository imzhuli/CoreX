#pragma once
#include "./core_value_util.hpp"

X_BEGIN

template <typename T>
class xVersion final : xNonCopyable {
private:
	using RawType = std::remove_cvref_t<T>;
	static_assert(!std::is_array_v<RawType>);
	static_assert(std::is_copy_constructible_v<RawType> || std::is_move_constructible_v<RawType>);

public:
	xVersion() = default;
	~xVersion() {
		TestAndDisable();
	}

	const RawType & Get() const {
		assert(IsEnabled());
		return *ValueHolder;
	}

	void Enable() {
		assert(!IsEnabled());
		ValueHolder.Create();
		EnableVersion();
	}

	template <typename... tArgs>
	void EnableValue(tArgs &&... Args) {
		assert(!IsEnabled());
		ValueHolder.CreateValue(std::forward<tArgs>(Args)...);
		EnableVersion();
	}

	void Disable() {
		assert(IsEnabled());
		DisableVersion();
		ValueHolder.Destroy();
	}

	void TestAndDisable() {
		if (!IsEnabled()) {
			return;
		}
		Disable();
	}

	bool IsEnabled() const {
		return !(Version & DISABLE_MASK);
	}

	int64_t GetVersion() const {
		return IsEnabled() ? Version : -1;
	}

	bool IsVersion(int64_t V) {
		return IsEnabled() && V == Version;
	}

private:
	static constexpr const int64_t DISABLE_MASK = 0x8000'0000'0000'0000;

	void EnableVersion() {
		++Version;
		Version &= ~DISABLE_MASK;
	}
	void DisableVersion() {
		Version |= DISABLE_MASK;
	}

private:
	xHolder<RawType> ValueHolder = {};
	int64_t          Version     = DISABLE_MASK;
};

X_END
