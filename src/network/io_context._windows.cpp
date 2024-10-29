#include "./io_context.hpp"

#ifdef X_SYSTEM_WINDOWS
X_BEGIN

void xIoContext::Interrupt() {
	PostQueuedCompletionStatus(Poller, 0, (ULONG_PTR) nullptr, nullptr);
}

bool xIoContext::CreatePoller() {
	assert(Poller == InvalidEventPoller);
	Poller = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	if (Poller == InvalidEventPoller) {
		return false;
	}
	auto PollerGuard = xScopeGuard([&] { CloseHandle(Steal(Poller, InvalidEventPoller)); });

	PollerGuard.Dismiss();
	return true;
}

void xIoContext::DestroyPoller() {
	CloseHandle(Steal(Poller, InvalidEventPoller));
}

bool xIoContext::Add(xSocketIoReactor & SocketReactor, bool Read, bool Write) {
	if (CreateIoCompletionPort((HANDLE)SocketReactor.GetNativeSocket(), Poller, (ULONG_PTR)&SocketReactor, 0) == NULL) {
		return false;
	}
	SocketReactor.IBP->Enabled = true;
	return true;
}

bool xIoContext::Update(xSocketIoReactor & SocketReactor, bool Read, bool Write) {
	Pure();
	return true;
}

void xIoContext::Remove(xSocketIoReactor & SocketReactor) {
	SocketReactor.IBP->Enabled = false;
	return;
}

void xIoContext::LoopOnce(int TimeoutMS) {
	OVERLAPPED_ENTRY EventEntries[256];
	ULONG            EventCount = 0;
	BOOL Result = GetQueuedCompletionStatusEx(Poller, EventEntries, (ULONG)Length(EventEntries), &EventCount, (TimeoutMS < 0 ? INFINITE : (DWORD)TimeoutMS), FALSE);

	if (!Result) {
		if (ERROR_ABANDONED_WAIT_0 == GetLastError()) {
			X_PFATAL("Invalid Poller");
		}
		return;
	}

	for (ULONG i = 0; i < EventCount; ++i) {
		auto & Event         = EventEntries[i];
		auto   OverlappedPtr = Event.lpOverlapped;

		// Get Outter object and see, if ioevent should be ignored:
		auto OverlappedBlockPtr = X_Entry(OverlappedPtr, xOverlappedObject, Overlapped);
		if (!OverlappedBlockPtr) {  // User Trigger event does not have overlapped object
			continue;
		}
		auto IBP = Release(OverlappedBlockPtr->Outter);
		if (!IBP || !IBP->Enabled) {  // object deleted
			X_DEBUG_PRINTF("Hit disabled ibp");
			continue;
		}

		auto ReactorPtr = IBP->Reactor;
		if (OverlappedBlockPtr == &IBP->Reader.Native) {
			ReactorPtr->EventFlags |= xIoReactor::IO_EVENT_READ;
			ReactorPtr->SetReadTransfered(Event.dwNumberOfBytesTransferred);
		} else if (OverlappedBlockPtr == &IBP->Writer.Native) {
			ReactorPtr->EventFlags |= xIoReactor::IO_EVENT_WRITE;
			ReactorPtr->SetWriteTransfered(Event.dwNumberOfBytesTransferred);
		} else {
			X_PFATAL("Invalid event");
		}

		EventList.GrabTail(ReactorPtr->EventNode);
	}

	ProcessEventList();
}

X_END
#endif
