#include "./core_time.hpp"

#ifdef X_SYSTEM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
namespace {
	[[maybe_unused]] static constexpr const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);
	[[maybe_unused]] X_STATIC_INLINE uint64_t        Internal_MicroTimestamp() {
        // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
        // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
        // until 00:00:00 January 1, 1970

        SYSTEMTIME system_time;
        FILETIME   file_time;
        uint64_t   time;

        GetSystemTime(&system_time);
        SystemTimeToFileTime(&system_time, &file_time);
        time  = ((uint64_t)file_time.dwLowDateTime);
        time += ((uint64_t)file_time.dwHighDateTime) << 32;
        return ((time - EPOCH) / 10L);
	}
}  // namespace
#else
#include <sys/time.h>
namespace {
	[[maybe_unused]] X_STATIC_INLINE uint64_t Internal_MicroTimestamp() {
		timeval tv;
		gettimeofday(&tv, nullptr);
		return tv.tv_sec * 1000000 + tv.tv_usec;
	}
}  // namespace
#endif

#if __cplusplus < 202002L && !defined(X_SYSTEM_WINDOWS)
X_BEGIN
uint64_t GetTimestampUS() {
	return Internal_MicroTimestamp();
}

static_assert(std::is_signed_v<xTimer::xDuration::rep>);

X_END
#endif