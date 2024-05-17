
#include "./io_context.hpp"

#ifdef X_SYSTEM_LINUX

#include <sys/eventfd.h>

#include <cinttypes>

X_BEGIN

bool xIoContext::xUserEventReactor::Init() {
	assert(UserEventFd == -1);
	UserEventFd = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
	if (-1 == UserEventFd) {
		return false;
	}
	return true;
}

void xIoContext::xUserEventReactor::Clean() {
	close(Steal(UserEventFd, -1));
}

bool xIoContext::xUserEventReactor::OnIoEventInReady() {
	uint64_t PseudoData;
	while (true) {
		auto Result = read(UserEventFd, &PseudoData, 8);
		if (Result < 0) {
			assert(EAGAIN == errno);
			break;
		}
		assert(Result == 8);
	}
	return true;
}

void xIoContext::Interrupt() {
	uint64_t PseudoData = 1;
	Ignore(write(UserEventReactor.UserEventFd, &PseudoData, 8));
}

bool xIoContext::CreatePoller() {
#if defined(X_SYSTEM_ANDROID) && X_SYSTEM_ANDROID < 23
	Poller = epoll_create(2048);
#else
	Poller = epoll_create1(EPOLL_CLOEXEC);
#endif
	if (Poller == InvalidEventPoller) {
		return false;
	}
	auto PollerGuard = xScopeGuard([this] { close(Steal(Poller, InvalidEventPoller)); });

	// setup user event fd:
	if (!UserEventReactor.Init()) {
		return false;
	}
	auto UserEventFdGuard = xScopeGuard([this] { UserEventReactor.Clean(); });

	struct epoll_event Event = {};
	Event.data.ptr           = &UserEventReactor;
	Event.events             = EPOLLET | EPOLLIN;
	if (-1 == epoll_ctl(Poller, EPOLL_CTL_ADD, UserEventReactor.UserEventFd, &Event)) {
		X_DEBUG_PRINTF("failed to register user event\n");
		return false;
	}

	Dismiss(PollerGuard, UserEventFdGuard);
	return true;
}

void xIoContext::DestroyPoller() {
	epoll_ctl(Poller, EPOLL_CTL_DEL, UserEventReactor.UserEventFd, nullptr);
	UserEventReactor.Clean();
	close(Steal(Poller, InvalidEventPoller));
}

bool xIoContext::Add(xSocketIoReactor & SocketReactor, bool Read, bool Write) {
	assert(!SocketReactor.EventFlags);

	auto Socket       = SocketReactor.GetNativeSocket();
	auto IoReactorPtr = static_cast<xIoReactor *>(&SocketReactor);

	struct epoll_event Event = {};
	Event.data.ptr           = IoReactorPtr;
	Event.events             = EPOLLET | EPOLLIN | (Write ? EPOLLOUT : 0);
	if (-1 == epoll_ctl(Poller, EPOLL_CTL_ADD, Socket, &Event)) {
		X_DEBUG_PRINTF("failed to register epoll event\n");
		return false;
	}
	return true;
}

bool xIoContext::Update(xSocketIoReactor & SocketReactor, bool Read, bool Write) {
	auto Socket       = SocketReactor.GetNativeSocket();
	auto IoReactorPtr = static_cast<xIoReactor *>(&SocketReactor);

	struct epoll_event Event = {};
	Event.data.ptr           = IoReactorPtr;
	Event.events             = EPOLLET | EPOLLIN | (Write ? EPOLLOUT : 0);
	if (-1 == epoll_ctl(Poller, EPOLL_CTL_MOD, Socket, &Event)) {
		X_PFATAL("failed to update epoll\n");
		return false;
	}
	return true;
}

void xIoContext::Remove(xSocketIoReactor & SocketReactor) {
	auto Socket = SocketReactor.GetNativeSocket();
	epoll_ctl(Poller, EPOLL_CTL_DEL, Socket, nullptr);

	EventList.Remove(SocketReactor.EventNode);
	SocketReactor.ResetReactorEvents();
}

void xIoContext::LoopOnce(int TimeoutMS) {
	struct epoll_event Events[128];
	int                Total = epoll_wait(Poller, Events, (int)Length(Events), TimeoutMS < 0 ? -1 : TimeoutMS);
	for (int i = 0; i < Total; ++i) {
		auto & EV         = Events[i];
		auto   ReactorPtr = (xIoReactor *)EV.data.ptr;
		if (EV.events & (EPOLLERR | EPOLLHUP)) {
			ReactorPtr->EventFlags |= xIoReactor::IO_EVENT_ERROR;
		} else {
			ReactorPtr->EventFlags |= ((EV.events & EPOLLIN) ? xIoReactor::IO_EVENT_READ : xIoReactor::IO_EVENT_NONE);
			ReactorPtr->EventFlags |= ((EV.events & EPOLLOUT) ? xIoReactor::IO_EVENT_WRITE : xIoReactor::IO_EVENT_NONE);
		}
		EventList.GrabTail(ReactorPtr->EventNode);
	}
	ProcessEventList();
}

X_END

#endif
