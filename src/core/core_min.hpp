// clang-format off
#pragma once

#include "./C/core_min.h"

#include <atomic>
#include <cassert>
#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

#define X_BEGIN namespace xel {
#define X_END }
#define X_COMMON_BEGIN namespace xel { inline namespace common {
#define X_COMMON_END }}

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

	constexpr const size_t InvalidDataSize = static_cast<size_t>(-1);

}  // namespace numeric

enum eBool : bool { 
	// intergral type could not be IMPLICITLY cast to eBool
	// that is safer for function default param values
	True  = true,
	False = false,
};

union xVariable {
	ubyte                      B[8];
	size_t                     Size;
	ptrdiff_t                  Offset;
	int                        I;
	unsigned int               U;
	float 	                   F;
	double                     D;
	void *                     P;
	const void *               CP;

	int8_t                     I8;
	int16_t                    I16;
	int32_t                    I32;
	int64_t                    I64;
	uint8_t                    U8;
	uint16_t                   U16;
	uint32_t                   U32;
	uint64_t                   U64;

	struct          { int32_t  IX, IY; };
	struct          { uint32_t UX, UY; };
	struct          { float    FX, FY; };
};
static_assert(sizeof(xVariable) == 8);
static_assert(std::is_trivially_copyable_v<xVariable>);

struct xPass final { public: void operator()() const {} };
struct xVBase { protected: constexpr xVBase() = default; virtual ~xVBase() = default; };
struct xAbstract { protected: constexpr xAbstract() = default; virtual ~xAbstract() = default; xAbstract(xAbstract &&) = delete; };
struct xNonCopyable { protected: constexpr xNonCopyable() = default; ~xNonCopyable() = default; xNonCopyable(xNonCopyable &&) = delete; };
struct xNonCatchable final { private: constexpr xNonCatchable() = default; ~xNonCatchable() = default; };

constexpr struct xNone final {} None{};
constexpr struct xNoInit final {} NoInit{};
constexpr struct xZeroInit final {} ZeroInit{};
constexpr struct xDefaultInit final {} DefaultInit{};
constexpr struct xGeneratorInit final {} GeneratorInit{};
constexpr struct xSizeInit final { size_t value; } ZeroSizeInit{};
constexpr struct xCapacityInit final { size_t value; } ZeroCapacityInit{};

template <typename T> // eXpiring object to Reference to object
[[nodiscard]] X_STATIC_INLINE std::remove_reference_t<T> & XR(T && ref) { return ref; }
template <typename T> // eXpiring object to Pointer to object
[[nodiscard]] X_STATIC_INLINE std::remove_reference_t<T> * XP(T && ref) { return &ref; }

template <typename T, size_t L>
[[nodiscard]] X_STATIC_INLINE constexpr size_t Length(const T (&)[L]) { return L; }
template <typename... Args>
[[nodiscard]] X_STATIC_INLINE constexpr size_t Count(const Args &... args) { return sizeof...(args); }
template <typename T>
X_STATIC_INLINE void ZeroFill(T* P, size_t L) { static_assert(!std::is_const_v<T> && std::is_trivially_constructible_v<T>); memset(P, 0, sizeof(*P) * L); }
template <typename T>
X_STATIC_INLINE std::enable_if_t<std::is_array_v<T>> ZeroFill(T &Array) { ZeroFill(Array, Length(Array)); }

[[noreturn]] X_API void QuickExit();
[[noreturn]] X_API void QuickExit(int ExitCode);
[[noreturn]] X_API void QuickExit(const char * PErrorMessage, int ExitCode = EXIT_FAILURE);
[[noreturn]] X_STATIC_INLINE void Error(const char * message) { QuickExit(message);}
[[noreturn]] X_STATIC_INLINE void Fatal(const char * message) { QuickExit(message); }
[[noreturn]] X_STATIC_INLINE void Todo(const char * info) { QuickExit(info); }
[[noreturn]] X_STATIC_INLINE void Pure() { QuickExit("Pure funcion placeholder called"); }
[[noreturn]] X_STATIC_INLINE void Unreachable() { QuickExit("Unreachable code is reached"); }

X_API void Breakpoint();
X_STATIC_INLINE void Pass(const auto &...) {}
X_STATIC_INLINE void RuntimeAssert(bool cond/* reason */, const char * hint_on_failure = "RuntimeAssertionFailure") { if (!cond) { Fatal(hint_on_failure); } }

template <typename T>
X_STATIC_INLINE constexpr auto MakeSigned(const T & Value) { return static_cast<std::make_signed_t<T>>(Value); }
template <typename T>
X_STATIC_INLINE constexpr auto MakeUnsigned(const T & Value) { return static_cast<std::make_unsigned_t<T>>(Value); }
template <typename T1, typename T0>
X_STATIC_INLINE constexpr auto Diff(const T1 & Value, const T0 & FromValue) { return Value - FromValue; }
template <typename T1, typename T0>
X_STATIC_INLINE constexpr auto SignedDiff(const T1 & Value, const T0 & FromValue) { return MakeSigned(Diff(Value, FromValue)); }
template <typename T1, typename T0>
X_STATIC_INLINE constexpr auto UnsignedDiff(const T1 & Value, const T0 & FromValue) { return MakeUnsigned(Diff(Value, FromValue)); }

template <typename T>
X_STATIC_INLINE constexpr bool IsDefaultValue(const T & Target) { return Target == T{}; }

template <typename T>
X_STATIC_INLINE T * ConstructAt(void * P) { new (P) T; return static_cast<T *>(P); }
template <typename T, typename... tArgs>
X_STATIC_INLINE T * ConstructAtWith(void * P, tArgs &&... Args) { new (P) T(std::forward<tArgs>(Args)...); return static_cast<T *>(P); }
template <typename T, typename... tArgs>
X_STATIC_INLINE T * ConstructAtWithList(void * P, tArgs &&... Args) { new (P) T{ std::forward<tArgs>(Args)... }; return static_cast<T *>(P); }
template <typename T>
X_STATIC_INLINE void DestructAt(void * P) { static_cast<T*>(P)->~T(); }

template <typename T>
X_STATIC_INLINE T * NoThrowConstructAt(void * P) noexcept {
	try { new (P) T; } catch (...) { return nullptr; }
	return static_cast<T *>(P);
}
template <typename T, typename... tArgs>
X_STATIC_INLINE T * NoThrowConstructAtWith(void * P, tArgs &&... Args) noexcept {
	try { new (P) T(std::forward<tArgs>(Args)...); } catch (...) { return nullptr; }
	return static_cast<T *>(P);
}
template <typename T, typename... tArgs>
X_STATIC_INLINE T * NoThrowConstructAtWithList(void * P, tArgs &&... Args) noexcept {
	try { new (P) T{ std::forward<tArgs>(Args)... }; } catch (...) { return nullptr; }
	return static_cast<T *>(P);
}
template <typename T>
X_STATIC_INLINE void NoThrowDestructAt(void * P) noexcept { static_cast<T*>(P)->~T(); }

template <typename T>
X_STATIC_INLINE constexpr void Reset(T & ExpiringTarget) { auto Zero = std::remove_cv_t<T>{}; std::swap(ExpiringTarget, Zero); }
template <typename T, typename TValue>
X_STATIC_INLINE constexpr void Reset(T & ExpiringTarget, TValue && value) { ExpiringTarget = std::forward<TValue>(value); }

template <typename T>
[[nodiscard]] X_STATIC_INLINE T Steal(T & ExpiringTarget) { T ret = std::move(ExpiringTarget); Reset(ExpiringTarget); return ret; }
template <typename T, typename TValue>
[[nodiscard]] X_STATIC_INLINE T Steal(T & ExpiringTarget, TValue && value) { T ret = std::move(ExpiringTarget); Reset(ExpiringTarget, std::forward<TValue>(value)); return ret; }


X_STATIC_INLINE constexpr const char * YN(bool y) { return y ? "yes" : "no"; }
X_STATIC_INLINE constexpr const char * TF(bool t) { return t ? "true" : "false"; }
template <typename T>
[[nodiscard]] X_STATIC_INLINE constexpr bool IsPow2(const T x) { static_assert(std::is_unsigned_v<T>); return x && !(x & (x - 1)); }

template <typename T>
class xRef final {
public:
	[[nodiscard]] constexpr explicit xRef(T & Ref) noexcept : _Ref(&Ref) {}
	[[nodiscard]] constexpr xRef(const xRef & RRef) noexcept = default;
	X_INLINE constexpr T & Get() const noexcept { return *_Ref; }
private:
	T * _Ref;
};

template <typename tFuncObj, typename... Args>
struct xInstantRun final : xNonCopyable {
	X_INLINE xInstantRun(tFuncObj && Func, Args &&... args) { std::forward<tFuncObj>(Func)(std::forward<Args>(args)...); }
};
template <typename tFuncObj, typename... Args>
xInstantRun(tFuncObj && Func, Args &&... args) -> xInstantRun<tFuncObj, Args...>;

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
	
	X_INLINE xValueGuard(xValueGuard & Other) = delete ;
	X_INLINE xValueGuard(xValueGuard && Other) = delete ;
	X_INLINE xValueGuard(const xValueGuard & Other) = delete ;

	X_INLINE ~xValueGuard() { if (_DismissExit) { return; } _Ref = _OldValue; }

	template<typename U>
	X_INLINE xValueGuard(T & Ref, const U & NewValue) : _Ref(Ref) { _OldValue = _Ref; _Ref = NewValue; }
	template<typename U>
	X_INLINE xValueGuard(T & Ref, U && NewValue) : _Ref(Ref) { _OldValue = _Ref; _Ref = std::move(NewValue); }

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
	X_INLINE ~xScopeGuard() { if (_DismissExit) { return; } _ExitCallback(); }
	X_INLINE void Dismiss() { _DismissExit = true; }
};
template <typename tEntry, typename tExit>
xScopeGuard(const tEntry & Entry, const tExit & Exit) -> xScopeGuard<std::decay_t<tEntry>, std::decay_t<tExit>>;
template <typename tExit>
xScopeGuard(const tExit & Exit) -> xScopeGuard<xPass, std::decay_t<tExit>>;
template <typename tEntry, typename tExit>
xScopeGuard(xScopeGuard<tEntry, tExit> && Other) -> xScopeGuard<tEntry, tExit>;

template <typename xTarget>
class xScopeCleaner final : xNonCopyable {
public:
	xScopeCleaner(xTarget & T) : Ref(T){}
	~xScopeCleaner() { if (!Dismissed) { Ref.Clean(); }}
	void Dismiss() { Dismissed = true; }
private:
	xTarget & Ref;
	bool Dismissed = false;
};

X_STATIC_INLINE void Dismiss() {}
template<typename T0, typename...T>
X_STATIC_INLINE void Dismiss(T0 & Guard0, T & ...Guards) {
	Guard0.Dismiss();
	Dismiss(Guards...);
}

template <typename T, bool DoThrow = false>
class xResourceGuard final : xNonCopyable {
public:
	template <typename... tArgs>
	X_INLINE constexpr xResourceGuard(T & Resource, tArgs &&... Args)
		: _Resource(Resource), _Inited(Resource.Init(std::forward<tArgs>(Args)...)) {
	}
	X_INLINE constexpr xResourceGuard(T && Other)
		: _Resource(Other._Resource), _Inited(Steal(Other._Inited)) {
	}
	X_INLINE ~xResourceGuard() {
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
template <typename T, typename... tArgs>
xResourceGuard(T & Resource, tArgs &&... Args) -> xResourceGuard<T>;

template <typename T>
class xConditionalResourceGuard final : xNonCopyable {
public:
    template <typename... tArgs>
    X_INLINE constexpr xConditionalResourceGuard(bool Cond, T & Resource, tArgs &&... Args) 
        : _Resource(Resource)
        , _Inited(Cond && Resource.Init(std::forward<tArgs>(Args)...)) 
    {}
    X_INLINE ~xConditionalResourceGuard() {
        if (_Inited) {
            _Resource.Clean();
        }
    }
    X_INLINE operator bool() const { return _Inited; }

private:
    T &        _Resource;
    const bool _Inited;
};
template <typename T, typename... tArgs>
xConditionalResourceGuard(bool Cond, T & Resource, tArgs &&... Args) -> xConditionalResourceGuard<T>;

class xRunState final {
public:
	X_INLINE bool Start()    { return _RunState.compare_exchange_strong(XR(NO_INSTANCE), RUNNING); }
	X_INLINE void Stop()     { _RunState.compare_exchange_strong(XR(RUNNING), STOPPING); }
	X_INLINE void Finish()   { _RunState = NO_INSTANCE; }
	X_INLINE operator bool() const { return _RunState == RUNNING; }
private:
	enum eState { NO_INSTANCE, RUNNING, STOPPING, };
	std::atomic<eState> _RunState = NO_INSTANCE;

    static_assert(decltype(_RunState)::is_always_lock_free);
};

namespace __common_detail__ {
	// force external linked object to enable checks in core_min.cpp
	struct xCompilerUnitEntry {
		X_API_MEMBER xCompilerUnitEntry();
	};
	[[maybe_unused]] inline xCompilerUnitEntry EntryObject;
}  // namespace __common_detail__

/*********************/

X_API void DebugPrintf(const char * Filename, size_t Line, const char * FunctionName, const char * fmt, ...);
X_API void ErrorPrintf(const char * Filename, size_t Line, const char * FunctionName, const char * fmt, ...);
X_API void FatalPrintf(const char * Filename, size_t Line, const char * FunctionName, const char * fmt, ...);

X_COMMON_END

#define X_CONCAT(a, b)              a##b
#define X_CONCAT_FORCE_EXPAND(a, b) X_CONCAT(a, b)

#ifndef X_SCOPE_GUARD
#define X_SCOPE_GUARD(...) auto X_CONCAT_FORCE_EXPAND(__X_ScopeGuard__, __LINE__) = ::xel::xScopeGuard(__VA_ARGS__)
#endif

#ifndef X_RESOURCE_GUARD
#define X_RESOURCE_GUARD(...)                                                                       \
    auto X_CONCAT_FORCE_EXPAND(__X_ResourceGuard__, __LINE__) = ::xel::xResourceGuard(__VA_ARGS__); \
    ::xel::RuntimeAssert(X_CONCAT_FORCE_EXPAND(__X_ResourceGuard__, __LINE__))
#endif

#ifndef X_VAR
#define X_VAR auto X_CONCAT_FORCE_EXPAND(__X_Variable__, __LINE__) =
#endif

#ifndef X_COND_GUARD
#define X_COND_GUARD(cond, ...)                                                                                                                             \
    auto X_CONCAT_FORCE_EXPAND(__X_Cond__, __LINE__)  = (bool)(cond);                                                                                       \
    auto X_CONCAT_FORCE_EXPAND(__X_ResourceGuard__, __LINE__) = ::xel::xConditionalResourceGuard(X_CONCAT_FORCE_EXPAND(__X_Cond__, __LINE__), __VA_ARGS__); \
    ::xel::RuntimeAssert(!X_CONCAT_FORCE_EXPAND(__X_Cond__, __LINE__) || X_CONCAT_FORCE_EXPAND(__X_ResourceGuard__, __LINE__))
#endif

#define X_PDEBUG(fmt, ...) ::xel::DebugPrintf(__FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define X_PERROR(fmt, ...) ::xel::ErrorPrintf(__FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)
#define X_PFATAL(fmt, ...) ::xel::FatalPrintf(__FILE__, __LINE__, __func__, fmt, ##__VA_ARGS__)

#define X_CATCH_NONE catch (const ::xel::xNonCatchable &)
#define X_RUNTIME_ASSERT(cond) ::xel::RuntimeAssert((cond), "RuntimeAssertionFailure@ " __FILE__ ":" X_STRINGIFY(__LINE__))

#ifndef NDEBUG
#define X_DEBUG
#define X_DEBUG_STEAL(Param, ...) ::xel::Steal(Param, ##__VA_ARGS__)
#define X_DEBUG_RESET(Param, ...) ::xel::Reset(Param, ##__VA_ARGS__)
#define X_DEBUG_PRINTF(...)  ::xel::DebugPrintf(__FILE__, __LINE__, __func__, "" __VA_ARGS__)
#define X_DEBUG_BREAKPOINT(...)   ::xel::Breakpoint()
#else
#define X_DEBUG_STEAL(Param, ...) Param
#define X_DEBUG_RESET(Param, ...)
#define X_DEBUG_PRINTF(...)  ::xel::Pass()
#define X_DEBUG_BREAKPOINT(...)
#endif
