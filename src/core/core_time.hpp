#pragma once
#include "./core_min.hpp"

#include <chrono>
#include <ctime>

X_BEGIN

using xHiResxClock     = std::chrono::high_resolution_clock;
using xHiResxTimePoint = xHiResxClock::time_point;
using xHiResDuration   = xHiResxClock::duration;

using xSteadyxClock     = std::conditional<xHiResxClock::is_steady, xHiResxClock, std::chrono::steady_clock>::type;
using xSteadyxTimePoint = xSteadyxClock::time_point;
using xSteadyDuration   = xSteadyxClock::duration;

using xSeconds      = std::chrono::seconds;
using xMilliSeconds = std::chrono::milliseconds;
using xMicroSeconds = std::chrono::microseconds;
using xNanoSeconds  = std::chrono::nanoseconds;

#if __cplusplus < 202002L && !defined(X_SYSTEM_WINDOWS)
X_API uint64_t GetTimestampUS();
#else
X_INLINE uint64_t GetTimestampUS() {
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
#endif
X_INLINE uint64_t GetTimestampMS() {
	return GetTimestampUS() / 1000;
}
X_INLINE int64_t GetUnixTimestamp() {  // the return time allows time before 1970, though time() may not
	return (int64_t)time(nullptr);
}

class xTimer {
public:
	using xClock     = xSteadyxClock;
	using xTimePoint = xSteadyxTimePoint;
	using xDuration  = xSteadyDuration;

	X_STATIC_INLINE xTimePoint Now() { return xClock::now(); }

private:
	xTimePoint _LastTagTime;

public:
	X_INLINE xTimer() { _LastTagTime = Now(); }

	X_INLINE xTimer(const xZeroInit &) { _LastTagTime = xTimePoint(); }

	X_INLINE xDuration Elapsed() { return Now() - _LastTagTime; }

	X_INLINE xDuration Skip(const xDuration & duration) {
		_LastTagTime += duration;
		return duration;
	}

	X_INLINE void Tag() { Tag(Now()); }

	X_INLINE void Tag(xTimePoint tp) { _LastTagTime = tp; }

	X_INLINE xTimePoint TagAndGet() {
		auto N = Now();
		Tag(N);
		return N;
	}

	X_INLINE xDuration TagAndGetElapsed() {
		auto L = _LastTagTime;
		auto N = Now();
		Tag(N);
		return N - L;
	}

	X_INLINE bool TestAndTag(xDuration testDuration) {
		xTimePoint n = Now();
		if (MakeSigned(Diff(n - _LastTagTime, testDuration).count()) >= 0) {
			_LastTagTime = n;
			return true;
		}
		return false;
	}

	X_INLINE xTimePoint GetAndTag() { return Steal(_LastTagTime, Now()); }

	/**
	 * @brief Consume:
	 *  Consume time by duration, return true if consuming is executed
	 *
	 *  Usage:
	 *  Sometimes, we expect no matter how much test is delayed,
	 *  some procedure must take place as if test happens continously, and by interval and no event is missed. (ex:
	 * physics engine)
	 *
	 *  Sample:
	 *  while(Consume(1s)) {
	 * 		// do something.
	 *  }
	 *  so even if the test happens 10s later, it acts as if test happeds every 1s, and 10 times in all.
	 */
	X_INLINE bool Consume(xDuration duration) {
		xTimePoint target = _LastTagTime + duration;
		if (MakeSigned(Diff(Now(), target).count()) >= 0) {
			_LastTagTime = target;
			return true;
		}
		return false;
	}
};

class xTicker final {
public:
	xTicker() { Update(); }
	xTicker(const xTicker &) = default;
	xTicker(const xZeroInit &) { NowMS = 0; }
	uint64_t Update() { return (NowMS = GetTimestampMS()); }
	void     Update(uint64_t TimestampMS) { NowMS = TimestampMS; }
	uint64_t operator()() const { return NowMS; }

private:
	uint64_t NowMS;
};

X_END
