#include "./timer_wheel.hpp"

#include "./core_time.hpp"

X_BEGIN

bool xTimerWheel::Init(size_t Total, uint64_t GapMS) {
	assert(!Lists);
	assert(!CurrentListIndex);
	assert(!TotalListNode);

	if (!(Lists = new (std::nothrow) xList<xListNode>[Total])) {
		return false;
	}
	CurrentListIndex = 0;
	TotalListNode    = Total;
	TimeGapMS        = GapMS ? GapMS : 1;
	NextTimestampMS  = GetTimestampMS() + TimeGapMS;
	return true;
}

void xTimerWheel::Clean() {
	assert(Lists);
	for (size_t i = 0; i < TotalListNode; ++i) {
		auto & L = Lists[i];
		while (L.PopHead()) {
			Pass();
		}
	}

	delete[] Lists;
	X_DEBUG_RESET(Lists, nullptr);
	X_DEBUG_RESET(CurrentListIndex, 0);
	X_DEBUG_RESET(TotalListNode, 0);
	X_DEBUG_RESET(TimeGapMS, 0);
	X_DEBUG_RESET(NextTimestampMS, 0);
}

void xTimerWheel::Forward() {
	uint64_t NowMS = GetTimestampMS();
	for (; NextTimestampMS <= NowMS; NextTimestampMS += TimeGapMS) {
		if (++CurrentListIndex >= TotalListNode) {
			CurrentListIndex = 0;
		}
		DispatchEvent(Lists[CurrentListIndex], NextTimestampMS);
	}
}

void xTimerWheel::ScheduleByOffset(xTimerWheelNode * NP, size_t Offset, TimerNodeCallback Callback) {
	assert(!xListNode::IsLinked(NP->Node) && !NP->Callback);
	assert(Offset < TotalListNode);
	auto Position = CurrentListIndex + Offset;
	if (Position >= TotalListNode) {
		Position -= TotalListNode;
		assert(Position < TotalListNode);
	}
	Lists[Position].AddTail(NP->Node);
}

void xTimerWheel::ScheduleByTimeoutMS(xTimerWheelNode * NP, uint64_t TimeoutMS, TimerNodeCallback Callback) {
	ScheduleByOffset(NP, (TimeoutMS + TimeGapMS - 1) / TimeGapMS, Callback);
}

void xTimerWheel::DispatchEvent(xList<xListNode> & List, uint64_t TimestampMS) {
	while (auto NP = List.PopHead()) {
		auto Real = X_Entry(NP, xTimerWheelNode, Node);
		X_DEBUG_STEAL(Real->Callback)(Real, TimestampMS);
	}
}

X_END
