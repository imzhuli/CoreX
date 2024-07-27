#include "./timer_wheel.hpp"

#include "./core_time.hpp"

X_BEGIN

bool xTimerWheel::Init(size_t Total, uint64_t GapMS) {
	assert(!Lists);
	assert(!CurrentListIndex);
	assert(!TotalListNode);
	assert(!MaxTimeout);

	auto WheelSize = Total + 1;
	assert(WheelSize);
	if (!(Lists = new (std::nothrow) xList<xListNode>[WheelSize])) {
		return false;
	}
	CurrentListIndex = 0;
	TotalListNode    = WheelSize;
	TimeGapMS        = GapMS ? GapMS : 1;
	NextTimestampMS  = GetTimestampMS() + TimeGapMS;
	MaxTimeout       = Total * TimeGapMS;
	return true;
}

void xTimerWheel::Clean() {
	assert(Lists);
	while (auto P = NextEventList.PopHead()) {
		Touch(P);
	}
	for (size_t i = 0; i < TotalListNode; ++i) {
		auto & L = Lists[i];
		while (auto P = L.PopHead()) {
			Touch(P);
		}
	}

	delete[] Lists;
	X_DEBUG_RESET(Lists, nullptr);
	X_DEBUG_RESET(CurrentListIndex, 0);
	X_DEBUG_RESET(TotalListNode, 0);
	X_DEBUG_RESET(TimeGapMS, 0);
	X_DEBUG_RESET(NextTimestampMS, 0);
	X_DEBUG_RESET(MaxTimeout, 0);
}

void xTimerWheel::Forward() {
	do {  // process: Next event list
		auto TL = xList<>();
		TL.GrabListTail(NextEventList);
		DispatchEvent(TL, NextTimestampMS);
	} while (false);
	DispatchEvent(Lists[CurrentListIndex], NextTimestampMS);

	uint64_t NowMS = GetTimestampMS();
	for (; NextTimestampMS <= NowMS; NextTimestampMS += TimeGapMS) {
		if (++CurrentListIndex >= TotalListNode) {
			CurrentListIndex = 0;
		}
		DispatchEvent(Lists[CurrentListIndex], NextTimestampMS);
	}
}

void xTimerWheel::ScheduleNext(xTimerWheelNode & NR) {
	assert(!xListNode::IsLinked(NR.Node) && NR.Callback.Function);
	NextEventList.AddTail(NR.Node);
}

void xTimerWheel::ScheduleByOffset(xTimerWheelNode & NR, size_t Offset) {
	assert(!xListNode::IsLinked(NR.Node) && NR.Callback.Function);
	assert(Offset < TotalListNode);
	auto Position = CurrentListIndex + Offset;
	if (Position >= TotalListNode) {
		Position -= TotalListNode;
	}
	Lists[Position].AddTail(NR.Node);
}

void xTimerWheel::ScheduleByTimeoutMS(xTimerWheelNode & NR, uint64_t TimeoutMS) {
	ScheduleByOffset(NR, (TimeoutMS + TimeGapMS - 1) / TimeGapMS);
}

void xTimerWheel::DispatchEvent(xList<xListNode> & List, uint64_t TimestampMS) {
	while (auto NP = List.PopHead()) {
		auto Real = X_Entry(NP, xTimerWheelNode, Node);
		Real->Callback.Function(Real->Callback.Context, TimestampMS);
	}
}

X_END
