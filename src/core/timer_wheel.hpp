#pragma once
#include "./core_min.hpp"
#include "./list.hpp"

X_BEGIN

class xTimerWheelNode;
class xTimerWheel;
using TimerNodeCallback = void (*)(xTimerWheelNode *, uint64_t TimestampMS);

class xTimerWheelNode {
private:
	friend class xTimerWheel;
	xListNode                  Node;
	TimerNodeCallback Callback X_DEBUG_INIT(nullptr);
};

class xTimerWheel : xNonCopyable {
public:
	X_INLINE xTimerWheel()  = default;
	X_INLINE ~xTimerWheel() = default;

	X_API_MEMBER bool Init(size_t Total, uint64_t GapMS = 0);
	X_API_MEMBER void Clean();
	X_API_MEMBER void Forward();
	X_API_MEMBER void ScheduleByOffset(xTimerWheelNode & NP, TimerNodeCallback Callback, size_t Offset = 1);
	X_API_MEMBER void ScheduleByTimeoutMS(xTimerWheelNode & NP, TimerNodeCallback Callback, uint64_t TimeoutMS);

	X_INLINE void Remove(xTimerWheelNode & NR) {
		xList<xListNode>::Remove(NR.Node);
		X_DEBUG_RESET(NR.Callback);
	};

private:
	X_API_MEMBER void DispatchEvent(xList<xListNode> & List, uint64_t TimestampMS);

private:
	xList<xListNode> * Lists X_DEBUG_INIT(nullptr);
	size_t CurrentListIndex  X_DEBUG_INIT(0);
	size_t TotalListNode     X_DEBUG_INIT(0);
	uint64_t TimeGapMS       X_DEBUG_INIT(0);
	uint64_t NextTimestampMS X_DEBUG_INIT(0);
};

X_END
