#include "./thread.hpp"

X_BEGIN

void xThreadSynchronizer::Acquire() {
	auto lock = std::unique_lock(_Mutex);
	++_TotalSize;
}

void xThreadSynchronizer::Release() {
	auto   lock	   = std::unique_lock(_Mutex);
	auto & context = _Coutnexts[_ActiveContext];
	if (--_TotalSize == context.xWaitingCount && _TotalSize != 0) {
		do {
			context.xWaitingCount = 0;
			std::swap(_ActiveContext, _OtherContext);
			lock.unlock();
		} while (false);
		context.xCondtion.notify_all();
	}
}

void xThreadSynchronizer::Synchronize() {
	auto   lock	   = std::unique_lock(_Mutex);
	auto & context = _Coutnexts[_ActiveContext];
	if (++context.xWaitingCount == _TotalSize) {
		do {
			context.xWaitingCount = 0;
			std::swap(_ActiveContext, _OtherContext);
			lock.unlock();
		} while (false);
		context.xCondtion.notify_all();
	} else {
		context.xCondtion.wait(lock, [&context] { return context.xWaitingCount == 0; });
	}
}

X_END
