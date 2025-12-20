
#include "./io_context.hpp"

#ifdef X_SYSTEM_DARWIN

#include <sys/event.h>
#include <sys/types.h>

#include <cinttypes>

X_BEGIN

void xIoContext::Interrupt() {
	struct kevent event;
	EV_SET(&event, 0, EVFILT_USER, 0, NOTE_TRIGGER, 0, NULL);
	Pass(kevent(Poller, &event, 1, NULL, 0, NULL));
}

bool xIoContext::CreatePoller() {
	Poller = kqueue();
	if (Poller == InvalidEventPoller) {
		return false;
	}
	auto PollerGuard = xScopeGuard([&] { close(Steal(Poller, InvalidEventPoller)); });

	struct kevent UserEvent = {};
	EV_SET(&UserEvent, 0, EVFILT_USER, EV_ADD | EV_CLEAR, NOTE_TRIGGER, 0, nullptr);
	if (-1 == kevent(Poller, &UserEvent, 1, nullptr, 0, nullptr)) {
		X_DEBUG_PRINTF("failed to register user event\n");
		return false;
	}

	PollerGuard.Dismiss();
	return true;
}

void xIoContext::DestroyPoller() {
	struct kevent UserEvent = {};
	EV_SET(&UserEvent, 0, EVFILT_USER, EV_DELETE, NOTE_TRIGGER, 0, nullptr);
	if (-1 == kevent(Poller, &UserEvent, 1, nullptr, 0, nullptr)) {
		X_DEBUG_PRINTF("failed to remove user event\n");
	}
	close(Steal(Poller, InvalidEventPoller));
}

bool xIoContext::Add(xSocketIoReactor & SocketReactor, bool Read, bool Write) {
	assert(!SocketReactor.EventFlags);

	auto Socket       = SocketReactor.GetNativeSocket();
	auto IoReactorPtr = static_cast<xIoReactor *>(&SocketReactor);

	auto ReadFlags  = EV_ADD | (Read ? EV_ENABLE : EV_DISABLE) | EV_CLEAR;
	auto WriteFlags = EV_ADD | (Write ? EV_ENABLE : EV_DISABLE) | EV_CLEAR;

	struct kevent Event[2] = {};
	EV_SET(&Event[0], Socket, EVFILT_READ, ReadFlags, 0, 0, IoReactorPtr);
	EV_SET(&Event[1], Socket, EVFILT_WRITE, WriteFlags, 0, 0, IoReactorPtr);
	if (-1 == kevent(Poller, Event, 2, nullptr, 0, nullptr)) {
		X_DEBUG_PRINTF("failed to register event\n");
		return false;
	}
	return true;
}

bool xIoContext::Update(xSocketIoReactor & SocketReactor, bool Read, bool Write) {
	auto Socket       = SocketReactor.GetNativeSocket();
	auto IoReactorPtr = static_cast<xIoReactor *>(&SocketReactor);

	auto ReadFlags  = (Read ? EV_ENABLE : EV_DISABLE) | EV_CLEAR;
	auto WriteFlags = (Write ? EV_ENABLE : EV_DISABLE) | EV_CLEAR;

	struct kevent Event[2] = {};
	EV_SET(&Event[0], Socket, EVFILT_READ, ReadFlags, 0, 0, IoReactorPtr);
	EV_SET(&Event[1], Socket, EVFILT_WRITE, WriteFlags, 0, 0, IoReactorPtr);
	if (-1 == kevent(Poller, Event, 2, nullptr, 0, nullptr)) {
		X_PFATAL("failed to update kevent\n");
		return false;
	}
	return true;
}

void xIoContext::Remove(xSocketIoReactor & SocketReactor) {
	auto Socket = SocketReactor.GetNativeSocket();

	struct kevent Event[2] = {};
	EV_SET(&Event[0], Socket, EVFILT_READ, EV_DELETE, 0, 0, nullptr);
	EV_SET(&Event[1], Socket, EVFILT_WRITE, EV_DELETE, 0, 0, nullptr);
	kevent(Poller, Event, 2, nullptr, 0, nullptr);

	EventList.Remove(SocketReactor.EventNode);
	SocketReactor.ResetReactorEvents();
}

void xIoContext::LoopOnce(int TimeoutMS) {
	ProcessEventList();  // process deferred operations after looponce
	struct kevent   Events[256];
	struct timespec TS = {
		TimeoutMS / 1000,
		((long)TimeoutMS % 1000) * 1000000,
	};
	int Total = kevent(Poller, NULL, 0, Events, Length(Events), TimeoutMS < 0 ? nullptr : &TS);
	for (int i = 0; i < Total; ++i) {
		auto & EV         = Events[i];
		auto   ReactorPtr = (xIoReactor *)EV.udata;
		if (!ReactorPtr) {  // user event
			continue;
		}
		if (EV.flags & EV_ERROR) {
			ReactorPtr->EventFlags |= xIoReactor::IO_EVENT_ERROR;
		} else {
			ReactorPtr->EventFlags |= ((EV.filter == EVFILT_READ) ? xIoReactor::IO_EVENT_READ : xIoReactor::IO_EVENT_NONE);
			ReactorPtr->EventFlags |= ((EV.filter == EVFILT_WRITE) ? xIoReactor::IO_EVENT_WRITE : xIoReactor::IO_EVENT_NONE);
		}
		EventList.GrabTail(ReactorPtr->EventNode);
	}
	ProcessEventList();
}

X_END

#endif
