#pragma once
#include "./core_min.hpp"

X_BEGIN

// class helping iterating an list or map,
// might be useful in some script language that accept cpp list or map as an input
template <typename IteratorType>
class xRangeIterator {

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

	X_INLINE xRangeIterator() = delete;
	X_INLINE constexpr xRangeIterator(const IteratorType & Begin, const IteratorType & End)
		: _Begin(Begin), _End(End) {
	}
	template <typename tContainer>
	X_INLINE constexpr xRangeIterator(tContainer & Container)
		: xRangeIterator(Container.begin(), Container.end()) {
	}
	template <typename tContainer>
	X_INLINE constexpr xRangeIterator(tContainer && Container)
		: xRangeIterator(Container.begin(), Container.end()) {
	}

	X_INLINE constexpr xRangeIterator(const xRangeIterator &)             = default;
	X_INLINE constexpr xRangeIterator(xRangeIterator &&)                  = default;
	X_INLINE constexpr xRangeIterator & operator=(const xRangeIterator &) = default;
	X_INLINE constexpr xRangeIterator & operator=(xRangeIterator &&)      = default;

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
xRangeIterator(const tWrapper &) -> xRangeIterator<typename tWrapper::iterator>;
template <typename tWrapper>
xRangeIterator(tWrapper &&) -> xRangeIterator<typename tWrapper::iterator>;

template <typename T>
class xArrayView final {
	static_assert(!std::is_const_v<T>);
	static_assert(!std::is_reference_v<T>);

public:
	X_INLINE xArrayView() = default;

	X_INLINE xArrayView(const T * start, const T * end)
		: _Start(start), _End(end), _Size(end - start) {
		assert(_Start <= _End);
	}

	template <typename N, typename = std::enable_if_t<std::is_integral_v<N> && !std::is_pointer_v<N>>>
	X_INLINE xArrayView(const T * start, N number)
		: _Start(start), _End(start + number), _Size(number) {
		assert(_Start <= _End);
	}

	template <size_t N>
	xArrayView(T (&start)[N])
		: _Start(start), _End(start + N), _Size(N) {
	}

	template <size_t N>
	xArrayView(const T (&start)[N])
		: _Start(start), _End(start + N), _Size(N) {
	}

	X_INLINE const T & operator[](ptrdiff_t off) const {
		return *(_Start + off);
	}

	X_INLINE const T * Data() const {
		return _Start;
	}

	X_INLINE size_t Size() const {
		return _Size;
	}

	// for iteration
	X_INLINE const T * begin() const {
		return _Start;
	}
	X_INLINE const T * end() const {
		return _End;
	}

private:
	const T * _Start{};
	const T * _End{};
	size_t    _Size{};
};

template <typename T>
class xSliceView final {
	static_assert(!std::is_const_v<T>);
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

	X_INLINE T & operator[](ptrdiff_t off) const {
		return *reinterpret_cast<T *>(_Start + off * _Stride);
	}
	X_INLINE size_t Stride() const {
		return _Stride;
	}
	X_INLINE size_t Size() const {
		return _Size;
	}

	// for iteration
	X_INLINE xIterator begin() const {
		return xIterator(_Start, _Stride);
	}
	X_INLINE xIterator end() const {
		return xIterator(_End, 0);
	}

private:
	IteratorMemory _Start{ nullptr };
	IteratorMemory _End{ nullptr };
	size_t         _Stride{ sizeof(T) };
	size_t         _Size{ 0 };
};

X_END
