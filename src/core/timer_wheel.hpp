#pragma once
#include "./core_min.hpp"
#include "./list.hpp"

X_BEGIN

class xTimerWheelNode;
class xTimerWheel;

using xTimerWheelCallbackFunction = void (*)(xVariable Context, uint64_t TimestampMS);
using xTimerWheelCallbackContext  = xVariable;

struct xTimerWheelCallback final {
	using xCallback = xTimerWheelCallbackFunction;
	using xContext  = xTimerWheelCallbackContext;

	X_INLINE xTimerWheelCallback() = default;
	X_INLINE xTimerWheelCallback(xCallback CB, xVariable UC = {})
		: Function(CB), Context(UC) {
	}
	X_INLINE void Execute(uint64_t Timestamp) const {
		(*Function)(Context, Timestamp);
	}

	xTimerWheelCallbackFunction Function;
	xTimerWheelCallbackContext  Context;
};

X_INLINE void SetCallback(xTimerWheelNode & Node, xTimerWheelCallback Callback);
class xTimerWheelNode {
private:
	friend void SetCallback(xTimerWheelNode &, xTimerWheelCallback);
	friend class xTimerWheel;

	xListNode           Node;
	xTimerWheelCallback Callback;
};
void SetCallback(xTimerWheelNode & Node, xTimerWheelCallback Callback) {
	Node.Callback = Callback;
}
using xTimerWheelNodeCleaner = void (*)(xVariable Context, xTimerWheelNode * Node);

/**
	xTimerWheel:
		schedule a callback at sometime later, by offset, or by timeout
		ScheduleNext():
			event is triggered next time Forward is called, no mater GapMS is reached
		ScheduleByOffset():
			when offset is not 0, event is scheduled at (offset * GapMS)
			when offset 0, immediate event is scheduled,
				note this behaves differently from ScheduleNext,
				that this allows event that need self iteration to change itself multiple times in one loop
		ScheduleByTimeoutMS():
			event is scheduled by timeout, if timeout is smaller than GapMS,
			event is scheduled by offset 1,
			that means this function never schedule immediate event.

 */
class xTimerWheel : xNonCopyable {
public:
	X_INLINE xTimerWheel()  = default;
	X_INLINE ~xTimerWheel() = default;

	X_API_MEMBER bool Init(size_t Total, uint64_t GapMS = 0);
	X_API_MEMBER void Clean(xTimerWheelNodeCleaner = nullptr, xVariable CleanerContext = {});
	X_API_MEMBER void Forward();
	X_API_MEMBER void ScheduleNext(xTimerWheelNode & NR);
	X_API_MEMBER void ScheduleByOffset(xTimerWheelNode & NR, size_t Offset = 1);
	X_API_MEMBER void ScheduleByTimeoutMS(xTimerWheelNode & NR, uint64_t TimeoutMS);

	X_INLINE uint64_t GetMaxTimeout() const {
		return MaxTimeout;
	}

	X_INLINE void ScheduleNext(xTimerWheelNode & NR, xTimerWheelCallback Callback) {
		SetCallback(NR, Callback);
		ScheduleNext(NR);
	}

	X_INLINE void ScheduleByOffset(xTimerWheelNode & NR, xTimerWheelCallback Callback, size_t Offset = 1) {
		SetCallback(NR, Callback);
		ScheduleByOffset(NR, Offset);
	}

	X_INLINE void ScheduleByTimeoutMS(xTimerWheelNode & NR, xTimerWheelCallback Callback, uint64_t TimeoutMS) {
		SetCallback(NR, Callback);
		ScheduleByTimeoutMS(NR, TimeoutMS);
	}

	X_INLINE void Remove(xTimerWheelNode & NR) {
		xList<xListNode>::Remove(NR.Node);
		X_DEBUG_RESET(NR.Callback);
	};

	X_INLINE void RescheduleNext(xTimerWheelNode & NR) {
		Remove(NR);
		ScheduleNext(NR);
	}

	X_INLINE void RescheduleNext(xTimerWheelNode & NR, xTimerWheelCallback Callback) {
		Remove(NR);
		ScheduleNext(NR, Callback);
	}

	X_INLINE void RescheduleByOffset(xTimerWheelNode & NR, size_t Offset = 1) {
		Remove(NR);
		ScheduleByOffset(NR, Offset);
	}
	X_INLINE void RescheduleByOffset(xTimerWheelNode & NR, xTimerWheelCallback Callback, size_t Offset = 1) {
		Remove(NR);
		ScheduleByOffset(NR, Callback, Offset);
	}

	X_INLINE void RescheduleByTimeoutMS(xTimerWheelNode & NR, uint64_t TimeoutMS) {
		Remove(NR);
		ScheduleByTimeoutMS(NR, TimeoutMS);
	}
	X_INLINE void RescheduleByTimeoutMS(xTimerWheelNode & NR, xTimerWheelCallback Callback, uint64_t TimeoutMS) {
		Remove(NR);
		ScheduleByTimeoutMS(NR, Callback, TimeoutMS);
	}

private:
	X_API_MEMBER void DispatchEvent(xList<xListNode> & List, uint64_t TimestampMS);

private:
	xList<>                  NextEventList;
	xList<> * Lists          X_DEBUG_INIT(nullptr);
	size_t CurrentListIndex  X_DEBUG_INIT(0);
	size_t TotalListNode     X_DEBUG_INIT(0);
	uint64_t TimeGapMS       X_DEBUG_INIT(0);
	uint64_t NextTimestampMS X_DEBUG_INIT(0);
	uint64_t MaxTimeout      X_DEBUG_INIT(0);
};

X_END
