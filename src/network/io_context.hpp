// clang-format off
#pragma once

#include "../core/core_min.hpp"
#include "../core/core_value_util.hpp"
#include "../core/list.hpp"
#include "./base.hpp"
#include "./io_reactor.hpp"

X_BEGIN

class xIoContext {
public:
	X_API_MEMBER bool Init();
	X_API_MEMBER void Clean();

	X_API_MEMBER void LoopOnce(int TimeoutMS = 50);
	X_API_MEMBER void Interrupt();

	// add listener
	X_API_MEMBER bool Add(xSocketIoReactor & SocketReactor, bool Read = true, bool Write = false);
	X_API_MEMBER bool Update(xSocketIoReactor & SocketReactor, bool Read = true, bool Write = false);
	X_API_MEMBER void Remove(xSocketIoReactor & SocketReactor);

	// user event trigger
	X_API_MEMBER void DeferRead(xIoReactor & Reactor);
	X_API_MEMBER void DeferWrite(xIoReactor & Reactor);
	X_API_MEMBER void DeferReadWrite(xIoReactor & Reactor);
	X_API_MEMBER void DeferError(xIoReactor & Reactor);

	X_API_MEMBER bool operator()() const;

private:
	X_API_MEMBER bool CreatePoller();
	X_API_MEMBER void DestroyPoller();
	X_API_MEMBER void ProcessEventList();

private:
	xEventPoller        Poller = InvalidEventPoller;
	xIoContextEventList EventList;

#ifdef X_SYSTEM_LINUX
	struct xUserEventReactor : xIoReactor, xNonCopyable {
		bool OnIoEventInReady() override;
		int  UserEventFd = -1;

		bool Init();
		void Clean();
	};
	xUserEventReactor UserEventReactor;
#endif
};

X_END
