#pragma once
#include "./core_min.hpp"

X_BEGIN

template <typename T>
class xView {
private:
	using Type = std::remove_reference_t<T>;

public:  // clang-format off
	X_INLINE xView()              = default;
	X_INLINE xView(const xView &) = default;
	X_INLINE xView(Type * Start, Type * End) : Start(Start), End(End) { assert(Start && End); }
	X_INLINE xView(Type * Start, size_t Count) : Start(Start) { assert(Start && Count); End = Start + Count; }

	X_INLINE Type * Data() const { return Start; }
	X_INLINE size_t Size() const { return End - Start; }

	X_INLINE explicit operator bool() const { return Start; }
	X_INLINE Type * begin() const { return Start; }
	X_INLINE Type * end() const { return End; }

private:
	Type * Start = nullptr;
	Type * End   = nullptr;
};

X_END
