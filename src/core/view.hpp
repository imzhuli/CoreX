#pragma once
#include "./core_min.hpp"

X_BEGIN

// class helping iterating an list or map,
// might be useful in some script language that accept cpp list or map as an input
template <typename IteratorType>
class xIteratorRange {

	static_assert(!std::is_reference_v<IteratorType>);
	template <typename tIterator>
	struct xIsPairReference : std::false_type {};
	template <typename tK, typename tV>
	struct xIsPairReference<std::pair<tK, tV> &> : std::true_type {};
	template <typename tK, typename tV>
	struct xIsPairReference<const std::pair<tK, tV> &> : std::true_type {};

public:
	using iterator                             = IteratorType;
	static constexpr const bool IsPairIterator = xIsPairReference<decltype(*std::declval<IteratorType>())>::value;

	X_INLINE xIteratorRange() = delete;
	X_INLINE constexpr xIteratorRange(const IteratorType & Begin, const IteratorType & End)
		: _Begin(Begin), _End(End) {
	}
	template <typename tContainer>
	X_INLINE constexpr xIteratorRange(tContainer & Container)
		: xIteratorRange(Container.begin(), Container.end()) {
	}
	template <typename tContainer>
	X_INLINE constexpr xIteratorRange(tContainer && Container)
		: xIteratorRange(Container.begin(), Container.end()) {
	}

	X_INLINE constexpr xIteratorRange(const xIteratorRange &)             = default;
	X_INLINE constexpr xIteratorRange(xIteratorRange &&)                  = default;
	X_INLINE constexpr xIteratorRange & operator=(const xIteratorRange &) = default;
	X_INLINE constexpr xIteratorRange & operator=(xIteratorRange &&)      = default;

	X_INLINE constexpr IteratorType begin() const {
		return _Begin;
	}
	X_INLINE constexpr IteratorType end() const {
		return _End;
	}
	X_INLINE constexpr auto size() const {
		return _End - _Begin;
	}

private:
	IteratorType _Begin;
	IteratorType _End;
};
template <typename tWrapper>
xIteratorRange(const tWrapper &) -> xIteratorRange<typename tWrapper::iterator>;
template <typename tWrapper>
xIteratorRange(tWrapper &&) -> xIteratorRange<typename tWrapper::iterator>;

template <typename T = void>
class xView final {
private:
	static_assert(!std::is_reference_v<T>);

public:
	X_INLINE xView()              = default;
	X_INLINE xView(const xView &) = default;
	X_INLINE xView(const T * pv, size_t sz)
		: _DataPtr(pv), _Size(sz) {
	}

	template <typename U, size_t N>
	X_INLINE xView(const U (&refArray)[N])
		: xView(refArray, N) {
	}

	X_INLINE const void * operator()() const {
		return _DataPtr;
	}

	X_INLINE const T * Data() const {
		return _DataPtr;
	}

	X_INLINE size_t Size() const {
		return _Size;
	}

private:
	const T * _DataPtr = nullptr;
	size_t    _Size    = 0;
};

template <typename T>
class xRangeView final {
	static_assert(!std::is_reference_v<T>);

public:
	X_INLINE xRangeView() = default;

	X_INLINE xRangeView(T * start, T * end)
		: _Start(start), _End(end), _Size(end - start) {
		assert(_Start <= _End);
	}

	template <typename N, typename = std::enable_if_t<std::is_integral_v<N> && !std::is_pointer_v<N>>>
	X_INLINE xRangeView(T * start, N number)
		: _Start(start), _End(start + number), _Size(number) {
		assert(_Start <= _End);
	}

	template <size_t N>
	xRangeView(T (&start)[N])
		: _Start(start), _End(start + N), _Size(N) {
	}

	X_INLINE T & operator[](ptrdiff_t off) const {
		return *(_Start + off);
	}
	X_INLINE T * begin() const {
		return _Start;
	}
	X_INLINE T * end() const {
		return _End;
	}
	X_INLINE size_t size() const {
		return _Size;
	}

private:
	T *    _Start{};
	T *    _End{};
	size_t _Size{};
};

template <typename T>
class xSliceView final {
	static_assert(!std::is_reference_v<T>);
	using IteratorMemory      = std::conditional_t<std::is_const_v<T>, const ubyte *, ubyte *>;
	using IteratorInitPointer = std::conditional_t<std::is_const_v<T>, const void *, void *>;

public:
	class xIterator {
	public:
		X_INLINE xIterator(IteratorInitPointer ptr, size_t stride)
			: _Memory(reinterpret_cast<IteratorMemory>(ptr)), _Stride(stride) {
		}

		X_INLINE xIterator(const xIterator &) = default;
		X_INLINE ~xIterator()                 = default;

		X_INLINE T * operator->() const {
			return reinterpret_cast<T *>(_Memory);
		}
		X_INLINE T & operator*() const {
			return *reinterpret_cast<T *>(_Memory);
		}

		X_INLINE xIterator & operator++() {
			_Memory += _Stride;
			return *this;
		}
		X_INLINE bool operator==(const xIterator & other) {
			return _Memory == other._Memory;
		}
		X_INLINE bool operator!=(const xIterator & other) {
			return _Memory != other._Memory;
		}

	private:
		IteratorMemory _Memory;
		size_t         _Stride;
	};

public:
	X_INLINE xSliceView() = default;

	X_INLINE xSliceView(T * start, size_t number, size_t stride = sizeof(T))
		: _Start(reinterpret_cast<IteratorMemory>(start)), _End(_Start + number * stride), _Stride(stride), _Size(number) {
	}

	X_INLINE xIterator begin() const {
		return xIterator(_Start, _Stride);
	}
	X_INLINE xIterator end() const {
		return xIterator(_End, 0);
	}

	X_INLINE T & operator[](ptrdiff_t off) const {
		return *reinterpret_cast<T *>(_Start + off * _Stride);
	}
	X_INLINE size_t stride() const {
		return _Stride;
	}
	X_INLINE size_t size() const {
		return _Size;
	}

private:
	IteratorMemory _Start{ nullptr };
	IteratorMemory _End{ nullptr };
	size_t         _Stride{ sizeof(T) };
	size_t         _Size{ 0 };
};

X_END
