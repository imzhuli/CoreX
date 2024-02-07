#pragma once
#include "./core_min.h"

#include <atomic>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

// clang-format off
X_COMMON_BEGIN

inline namespace numeric {
	using byte  = ::std::byte;
	using ubyte = unsigned char;

	using size8_t  = ::std::uint8_t;
	using size16_t = ::std::uint16_t;
	using size32_t = ::std::uint32_t;
	using size64_t = ::std::uint64_t;

	using ssize8_t  = ::std::int8_t;
	using ssize16_t = ::std::int16_t;
	using ssize32_t = ::std::int32_t;
	using ssize64_t = ::std::int64_t;

	using offset8_t  = ::std::int8_t;
	using offset16_t = ::std::int16_t;
	using offset32_t = ::std::int32_t;
	using offset64_t = ::std::int64_t;
	using offset_t   = ::std::ptrdiff_t;

	using size_t  = ::std::size_t;
	using ssize_t = typename ::std::make_signed<size_t>::type;

}  // namespace numeric

union xVariable {
	ubyte B[8];

	void *       Pointer;
	const void * ConstPointer;

	ptrdiff_t Offset;
	size_t    Size;

	int          I;
	unsigned int U;
	int8_t       I8;
	int16_t      I16;
	int32_t      I32;
	int64_t      I64;
	uint8_t      U8;
	uint16_t     U16;
	uint32_t     U32;
	uint64_t     U64;
	struct {
		int32_t X, Y;
	} IV2;
	struct {
		uint32_t X, Y;
	} UV2;
};

struct xVersion {
	uint16_t Major;
	uint16_t Minor;
	uint16_t Build;
	uint16_t Revision;
};

struct xPass final { X_INLINE void operator()() const {} };
struct xVBase { protected: constexpr xVBase() = default; virtual ~xVBase()  = default; };
struct xAbstract { protected: constexpr xAbstract() = default; virtual ~xAbstract() = default; xAbstract(xAbstract &&) = delete; };
struct xNonCopyable { protected: constexpr xNonCopyable() = default; ~xNonCopyable() = default; xNonCopyable(xNonCopyable &&) = delete; };
struct xNonCatchable final { private: constexpr xNonCatchable() = default; ~xNonCatchable() = default; };

constexpr struct xNone final {} None;
constexpr struct xNoInit final {} NoInit{};
constexpr struct xZeroInit final {} ZeroInit{};
constexpr struct xDefaultInit final {} DefaultInit{};
constexpr struct xGeneratorInit final {} GeneratorInit{};
constexpr struct xSizeInit final { size_t value; } ZeroSizeInit{};
constexpr struct xCapacityInit final { size_t value; } ZeroCapacityInit{};

template <typename T>  // std::in_place_type_t
struct xInPlaceType final { explicit constexpr xInPlaceType() = default;};
template <typename T>
inline constexpr xInPlaceType<T> const Type{};

template <typename T>
[[nodiscard]] X_STATIC_INLINE std::remove_reference_t<T> & X2Ref(T && ref) { return ref; }
template <typename T>
[[nodiscard]] X_STATIC_INLINE std::remove_reference_t<T> * X2Ptr(T && ref) { return &ref; }

template <typename T>
[[nodiscard]] X_STATIC_INLINE constexpr std::conditional_t<std::is_const_v<T>, const void *, void *> AddressOf(T & obj) {
	return &reinterpret_cast<std::conditional_t<std::is_const_v<T>, const unsigned char, unsigned char> &>(obj);
}
template <typename T, size_t L>
[[nodiscard]] X_STATIC_INLINE constexpr size_t Length(const T (&)[L]) { return L; }
template <typename T, size_t L>
[[nodiscard]] X_STATIC_INLINE constexpr size_t SafeLength(const T (&)[L]) { return L ? L - 1 : 0; }
template <typename... Args>
[[nodiscard]] X_STATIC_INLINE constexpr size_t Count(const Args &... args) { return sizeof...(args); }

[[noreturn]] X_STATIC_INLINE void Error(const char * message = nullptr) { throw message;}
[[noreturn]] X_STATIC_INLINE void Fatal(const char * = nullptr /* reason */) { std::abort(); }
[[noreturn]] X_STATIC_INLINE void Todo(const char * info = nullptr) { Fatal(info); }
[[noreturn]] X_STATIC_INLINE void Pure() { Fatal("placeholder of pure function called, which is not expected"); }

X_STATIC_INLINE void Pass(const char * = nullptr /* reason */) { }
X_API void Breakpoint();

template <typename T>
X_STATIC_INLINE constexpr auto MakeSigned(T && Value) { return static_cast<std::make_signed_t<std::remove_cvref_t<T>>>(std::forward<T>(Value)); }
template <typename T>
X_STATIC_INLINE constexpr auto MakeUnsigned(const T & Value) { return static_cast<std::make_unsigned_t<std::remove_cvref_t<T>>>(std::forward<T>(Value)); }
template <typename T1, typename T2>
X_STATIC_INLINE constexpr auto Diff(T1 && Value, T2 && FromValue) { return std::forward<T1>(Value) - std::forward<T2>(FromValue); }
template <typename T1, typename T2>
X_STATIC_INLINE constexpr auto SignedDiff(T1 && Value, T2 && FromValue) { return MakeSigned(Diff(std::forward<T1>(Value), std::forward<T2>(FromValue))); }
template <typename T1, typename T2>
X_STATIC_INLINE constexpr auto UnignedDiff(T1 && Value, T2 && FromValue) { return MakeUnsigned(Diff(std::forward<T1>(Value), std::forward<T2>(FromValue))); }

template <typename T>
X_STATIC_INLINE constexpr bool IsDefaultValue(const T & Target) { return Target == T{}; }

template <typename T, typename TValue>
X_STATIC_INLINE constexpr void Assign(T & ExpiringTarget, TValue && value) { ExpiringTarget = std::forward<TValue>(value); }
template <typename T>
X_STATIC_INLINE constexpr void Reset(T & ExpiringTarget) { ExpiringTarget = T(); }
template <typename T, typename TValue>
X_STATIC_INLINE constexpr void Reset(T & ExpiringTarget, TValue && value) { ExpiringTarget = std::forward<TValue>(value); }

template <typename T>
X_STATIC_INLINE void Construct(T & ExpiringTarget) { new ((void *)&ExpiringTarget) T; }
template <typename T, typename... tArgs>
X_STATIC_INLINE void ConstructValue(T & ExpiringTarget, tArgs &&... Args) { new ((void *)&ExpiringTarget) T(std::forward<tArgs>(Args)...); }
template <typename T, typename... tArgs>
X_STATIC_INLINE void ConstructValueWithList(T & ExpiringTarget, tArgs &&... Args) { new ((void *)&ExpiringTarget) T{ std::forward<tArgs>(Args)... }; }
template <typename T>
X_STATIC_INLINE void Destruct(T & ExpiringTarget) { ExpiringTarget.~T(); }

template <typename T>
X_STATIC_INLINE void Renew(T & ExpiringTarget) { ExpiringTarget.~T(); new ((void *)&ExpiringTarget) T; }
template <typename T, typename... tArgs>
X_STATIC_INLINE void RenewValue(T & ExpiringTarget, tArgs &&... Args) { ExpiringTarget.~T(); new ((void *)&ExpiringTarget) T(std::forward<tArgs>(Args)...); }
template <typename T, typename... tArgs>
X_STATIC_INLINE void RenewValueWithList(T & ExpiringTarget, tArgs &&... Args) { ExpiringTarget.~T(); new ((void *)&ExpiringTarget) T{ std::forward<tArgs>(Args)... }; }

template <typename T>
[[nodiscard]] X_STATIC_INLINE T Steal(T & ExpiringTarget) { T ret = std::move(ExpiringTarget); ExpiringTarget = T(); return ret; }
template <typename T, typename TValue>
[[nodiscard]] X_STATIC_INLINE T Steal(T & ExpiringTarget, TValue && value) { T ret = std::move(ExpiringTarget); ExpiringTarget = std::forward<TValue>(value); return ret; }


X_STATIC_INLINE constexpr const char * YN(bool y) { return y ? "yes" : "no"; }
X_STATIC_INLINE constexpr const char * TF(bool t) { return t ? "true" : "false"; }
template <typename T>
[[nodiscard]] X_STATIC_INLINE constexpr bool IsPow2(const T x) { static_assert(std::is_unsigned_v<T>); return !(x & (x - 1)); }

template <typename T>
class xRef final {
public:
	[[nodiscard]] constexpr explicit xRef(T & Ref) noexcept : _Ref(&Ref) {}
	[[nodiscard]] constexpr xRef(const xRef & RRef) noexcept = default;
	X_INLINE constexpr T & Get() const noexcept { return *_Ref; }
private:
	T * _Ref;
};

template <typename RefedT>
struct xRefCaster {
	static_assert(!std::is_reference_v<RefedT>);
	using Type = RefedT;
	X_STATIC_INLINE RefedT & Get(RefedT & R) { return R; }
	X_STATIC_INLINE const RefedT & Get(const RefedT & R) { return R; }
};

template <typename RefedT>
struct xRefCaster<xRef<RefedT>> {
	static_assert(!std::is_reference_v<RefedT>);
	using Type = RefedT;
	X_STATIC_INLINE RefedT & Get(const xRef<RefedT> & RR) { return RR.Get(); }
};

template <typename tFuncObj, typename... Args>
struct xInstantRun final : xNonCopyable {
	X_INLINE xInstantRun(tFuncObj && Func, Args &&... args) { std::forward<tFuncObj>(Func)(std::forward<Args>(args)...); }
};

template <typename T>
class xValueGuard final : xNonCopyable {
private:
	static_assert(!std::is_reference_v<T> && !std::is_const_v<T>);
	using xStorage = std::remove_cvref_t<T>;

	T &      _Ref;
	xStorage _OldValue;
	bool     _DismissExit = false;

public:
	X_INLINE xValueGuard(T & Ref) : _Ref(Ref) { _OldValue = _Ref; }
	X_INLINE xValueGuard(T & Ref, const T & NewValue) : _Ref(Ref) { _OldValue = _Ref; _Ref = NewValue; }
	X_INLINE xValueGuard(T & Ref, T && NewValue) : _Ref(Ref) { _OldValue = _Ref; _Ref = std::move(NewValue); }
	X_INLINE xValueGuard(xValueGuard && Other) : _Ref(Other._Ref), _OldValue(std::move(Other._OldValue)), _DismissExit(Steal(Other._DismissExit, true)) { }
	X_INLINE ~xValueGuard() { if (_DismissExit) { return; } _Ref = _OldValue; }

	X_INLINE const xStorage & operator()() const { return _OldValue; }
	X_INLINE void Dismiss() { _DismissExit = true; }
};

template <typename tEntry, typename tExit>
class xScopeGuard final : xNonCopyable {
private:
	/** NOTE: It's important typeof(_ExitCallback) is not reference,
	 *  so that it be compatible with:
	 *     function,
	 *     lambda (w/o worrying about capturing-lambda function's lifetime),
	 *     and func-object (which is often with inline trivial ctor(default/copy/move) and dtor).
	 *  if caller is quite aware of the lifetime of a func-object and if:
	 *       the fuct-object is non-copyable, or
	 *       avoiding ctor/copy ctor/dtor really matters
	 *     use xRef(some_non_const_object) above as a const-wrapper-object
	 * */
	tExit _ExitCallback;
	bool  _DismissExit = false;

public:
	[[nodiscard]] X_INLINE xScopeGuard(const tEntry & Entry, const tExit & Exit) : _ExitCallback(Exit) { Entry(); }
	[[nodiscard]] X_INLINE xScopeGuard(const tExit & Exit) : _ExitCallback(Exit) { }
	[[nodiscard]] X_INLINE xScopeGuard(xScopeGuard && Other) : _ExitCallback(Other._ExitCallback), _DismissExit(Steal(Other._DismissExit, true)) { }
	X_INLINE ~xScopeGuard() { if (_DismissExit) { return; } xRefCaster<tExit>::Get(_ExitCallback)(); }
	X_INLINE void Dismiss() { _DismissExit = true; }
};
template <typename tEntry, typename tExit>
xScopeGuard(const tEntry & Entry, const tExit & Exit) -> xScopeGuard<std::decay_t<tEntry>, std::decay_t<tExit>>;
template <typename tExit>
xScopeGuard(const tExit & Exit) -> xScopeGuard<xPass, std::decay_t<tExit>>;
template <typename tEntry, typename tExit>
xScopeGuard(xScopeGuard<tEntry, tExit> && Other) -> xScopeGuard<tEntry, tExit>;

namespace __common_detail__ {
	template <typename T, bool DoThrow = false>
	class xResourceGuardBase : xNonCopyable {
	public:
		template <typename... tArgs>
		X_INLINE constexpr xResourceGuardBase(T & Resource, tArgs &&... Args)
			: _Resource(Resource), _Inited(Resource.Init(std::forward<tArgs>(Args)...)) {
			if constexpr (DoThrow) {
				if (!_Inited) {
					throw "xResourceGuardBase failed to init resource";
				}
			}
		}
		X_INLINE constexpr xResourceGuardBase(T && Other)
			: _Resource(Other._Resource), _Inited(Steal(Other._Inited)) {
		}
		X_INLINE ~xResourceGuardBase() {
			if (_Inited) {
				_Resource.Clean();
			}
		}
		X_INLINE operator bool() const {
			return _Inited;
		}

	private:
		T &        _Resource;
		const bool _Inited;
	};
}  // namespace __common_detail__

template <typename T>
struct xResourceGuard final 
: __common_detail__::xResourceGuardBase<T, false> {
	using __common_detail__::xResourceGuardBase<T, false>::xResourceGuardBase;
};
template <typename T, typename... tArgs>
xResourceGuard(T & Resource, tArgs &&... Args) -> xResourceGuard<T>;

template <typename T>
struct xResourceGuardThrowable final 
: __common_detail__::xResourceGuardBase<T, true> {
	using __common_detail__::xResourceGuardBase<T, true>::xResourceGuardBase;
};
template <typename T, typename... tArgs>
xResourceGuardThrowable(T & Resource, tArgs &&... Args) -> xResourceGuardThrowable<T>;

namespace __common_detail__ {
	template <bool IsAtomic = false>
	class xReentryFlag final : xNonCopyable {
	private:
		using xCounter = std::conditional_t<IsAtomic, std::atomic_uint64_t, uint64_t>;
		class xGuard : xNonCopyable {
		public:
			xGuard(xCounter & CounterRef)
				: _CounterRef(CounterRef), _Entered(!CounterRef++){};
			xGuard(xGuard && Other)
				: _CounterRef(Other._CounterRef), _Entered(Steal(Other._Entered)) {
				++_CounterRef;
			};
			~xGuard() {
				--_CounterRef;
			}
			operator bool() const {
				return _Entered;
			}

		private:
			xCounter & _CounterRef;
			bool       _Entered;
			friend class xReentryFlag;
		};

	public:
		xGuard Guard() {
			return xGuard(_Counter);
		}

	private:
		xCounter _Counter{};
	};
}  // namespace __common_detail__
using xReentryFlag       = __common_detail__::xReentryFlag<false>;
using xAtomicReentryFlag = __common_detail__::xReentryFlag<true>;

template <typename xTarget>
auto MakeResourceCleaner(xTarget & Target) {
	return xScopeGuard([&Target] { Target.Clean(); });
}

X_API void DebugPrintf(const char * Filename, size_t Line, const char * FunctionName, const char * fmt, ...);

X_COMMON_END

#ifndef X_CATCH_NONE
#define X_CATCH_NONE catch (const ::xel::xNonCatchable &)
#endif

#ifndef NDEBUG
#define X_DEBUG
#define X_DEBUG_INIT(...)         = { __VA_ARGS__ }
#define X_DEBUG_STEAL(Param, ...) (::xel::Steal(Param, ##__VA_ARGS__))
#define X_DEBUG_RESET(Param, ...) (::xel::Reset(Param, ##__VA_ARGS__))
#define X_DEBUG_PRINTF(fmt, ...)  ::xel::DebugPrintf(__FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define X_DEBUG_FPRINTF           fprintf
#define X_DEBUG_BREAKPOINT(...)   ::xel::Breakpoint()
#else
#define X_DEBUG_INIT(...)
#define X_DEBUG_STEAL(Param, ...) Param
#define X_DEBUG_RESET(Param, ...)
#define X_DEBUG_PRINTF(...)  ::xel::Pass()
#define X_DEBUG_FPRINTF(...) ::xel::Pass()
#define X_DEBUG_BREAKPOINT(...)
#endif

#ifdef X_AddressOf
#undef X_AddressOf
#define X_AddressOf ::xel::AddressOf
#endif