#include "./io_context.hpp"

#ifdef X_SYSTEM_WINDOWS
X_BEGIN

void xIoContext::Interrupt() {
    PostQueuedCompletionStatus(Poller, 0, (ULONG_PTR)nullptr, nullptr);
}

bool xIoContext::CreatePoller() {
    assert(Poller == InvalidEventPoller);
    Poller = CreateIoCompletionPort(INVALID_HANDLE_VALUE,0,0,0);
	if (Poller == InvalidEventPoller) {
		return false;
	}
    auto PollerGuard = xScopeGuard([&]{
        CloseHandle(Steal(Poller, InvalidEventPoller));
    });

	PollerGuard.Dismiss();
	return true;
}

void xIoContext::DestroyPoller() {
    CloseHandle(Steal(Poller, InvalidEventPoller));
}


bool xIoContext::Add(xSocketIoReactor & SocketReactor, bool Read, bool Write) {
    return true;
	// assert(!SocketReactor.EventFlags);

	// auto Socket       = SocketReactor.GetNativeSocket();
	// auto IoReactorPtr = static_cast<xIoReactor *>(&SocketReactor);

	// struct epoll_event Event = {};
	// Event.data.ptr           = IoReactorPtr;
	// Event.events             = EPOLLET | EPOLLIN | (Write ? EPOLLOUT : 0);
	// if (-1 == epoll_ctl(Poller, EPOLL_CTL_ADD, Socket, &Event)) {
	// 	X_DEBUG_PRINTF("failed to register epoll event\n");
	// 	return false;
	// }
	// return true;
}

bool xIoContext::Update(xSocketIoReactor & SocketReactor, bool Read, bool Write) {
    return false;
	// auto Socket       = SocketReactor.GetNativeSocket();
	// auto IoReactorPtr = static_cast<xIoReactor *>(&SocketReactor);

	// struct epoll_event Event = {};
	// Event.data.ptr           = IoReactorPtr;
	// Event.events             = EPOLLET | EPOLLIN | (Write ? EPOLLOUT : 0);
	// if (-1 == epoll_ctl(Poller, EPOLL_CTL_MOD, Socket, &Event)) {
	// 	X_PFATAL("failed to update epoll\n");
	// 	return false;
	// }
	// return true;
}

void xIoContext::Remove(xSocketIoReactor & SocketReactor) {
	// auto Socket = SocketReactor.GetNativeSocket();
	// epoll_ctl(Poller, EPOLL_CTL_DEL, Socket, nullptr);

	// EventList.Remove(SocketReactor.EventNode);
	// SocketReactor.ResetReactorEvents();
}

void xIoContext::LoopOnce(int TimeoutMS) {
    Todo();
	// struct epoll_event Events[128];
	// int                Total = epoll_wait(Poller, Events, (int)Length(Events), TimeoutMS < 0 ? -1 : TimeoutMS);
	// for (int i = 0; i < Total; ++i) {
	// 	auto & EV         = Events[i];
	// 	auto   ReactorPtr = (xIoReactor *)EV.data.ptr;
	// 	if (EV.events & (EPOLLERR | EPOLLHUP)) {
	// 		ReactorPtr->EventFlags |= xIoReactor::IO_EVENT_ERROR;
	// 	} else {
	// 		ReactorPtr->EventFlags |= ((EV.events & EPOLLIN) ? xIoReactor::IO_EVENT_READ : xIoReactor::IO_EVENT_NONE);
	// 		ReactorPtr->EventFlags |= ((EV.events & EPOLLOUT) ? xIoReactor::IO_EVENT_WRITE : xIoReactor::IO_EVENT_NONE);
	// 	}
	// 	EventList.GrabTail(ReactorPtr->EventNode);
	// }
	ProcessEventList();
}

X_END
#endif
