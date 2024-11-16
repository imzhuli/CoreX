#include "./io_context.hpp"

X_BEGIN

bool xIoContext::Init() {
	if (!CreatePoller()) {
		return false;
	}
	return true;
}

void xIoContext::Clean() {
	DestroyPoller();
}

bool xIoContext::operator()() const {
	return Poller != InvalidEventPoller;
}

void xIoContext::DeferRead(xIoReactor & Reactor) {
	Reactor.EventFlags |= xIoReactor::IO_EVENT_READ;
	EventList.GrabTail(Reactor.EventNode);
}

void xIoContext::DeferWrite(xIoReactor & Reactor) {
	Reactor.EventFlags |= xIoReactor::IO_EVENT_WRITE;
	EventList.GrabTail(Reactor.EventNode);
}

void xIoContext::DeferReadWrite(xIoReactor & Reactor) {
	Reactor.EventFlags |= (xIoReactor::IO_EVENT_READ | xIoReactor::IO_EVENT_WRITE);
	EventList.GrabTail(Reactor.EventNode);
}

void xIoContext::DeferError(xIoReactor & Reactor) {
	Reactor.EventFlags |= xIoReactor::IO_EVENT_ERROR;
	EventList.GrabTail(Reactor.EventNode);
}

void xIoContext::ProcessEventList() {
	auto ProcessList = xIoContextEventList();
	ProcessList.GrabListTail(EventList);
	while (auto NP = ProcessList.PopHead()) {
		auto RP = static_cast<xIoReactor *>(X_Entry(NP, __network_detail__::__xIoReactor__, EventNode));
		if (RP->EventFlags & xIoReactor::IO_EVENT_DISABLED) {
			continue;
		}
		if (RP->EventFlags & xIoReactor::IO_EVENT_ERROR) {
			RP->OnIoEventError();
			RP->EventFlags = xIoReactor::IO_EVENT_DISABLED;
			continue;
		}
		if (RP->EventFlags & xIoReactor::IO_EVENT_READ) {
			if (!RP->OnIoEventInReady()) {
				RP->OnIoEventError();
				RP->EventFlags = xIoReactor::IO_EVENT_DISABLED;
				continue;
			}
		}
		if (RP->EventFlags & xIoReactor::IO_EVENT_WRITE) {
			if (!RP->OnIoEventOutReady()) {
				RP->OnIoEventError();
				RP->EventFlags = xIoReactor::IO_EVENT_DISABLED;
				continue;
			}
		}
		RP->EventFlags = xIoReactor::IO_EVENT_NONE;
	}
}

X_END
