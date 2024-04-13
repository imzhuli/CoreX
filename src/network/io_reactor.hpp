#pragma once
#include "../core/core_min.hpp"
#include "../core/core_value_util.hpp"
#include "../core/list.hpp"
#include "./base.hpp"

X_BEGIN

class xIoContext;
struct xIoContextEventNode : xListNode {};
using xIoContextEventList = xList<xIoContextEventNode>;

namespace __network_detail__ {
	struct __xIoReactor__ {
		static constexpr const uint32_t IO_EVENT_NONE  = 0x00;
		static constexpr const uint32_t IO_EVENT_ERROR = 0x01;
		static constexpr const uint32_t IO_EVENT_READ  = 0x02;
		static constexpr const uint32_t IO_EVENT_WRITE = 0x04;
		// IO_EVENT_DISABLED: reactor is disabled or error has been processed.
		// NOTE: this flag is never cleared
		static constexpr const uint32_t IO_EVENT_DISABLED = 0x08;

		uint32_t            EventFlags = IO_EVENT_NONE;
		xIoContextEventNode EventNode;
	};

}  // namespace __network_detail__

class xIoReactor : private __network_detail__::__xIoReactor__ {
	friend class xIoContext;

private:
	// clang-format off
	X_INLINE virtual void OnIoEventError() { Pure(); }
	X_INLINE virtual bool OnIoEventInReady() { return false; }
	X_INLINE virtual bool OnIoEventOutReady() { return false; }
	// clang-format on
protected:
	X_INLINE void ResetReactorEvents() {
		// xListNode::Unlink(EventNode);
		assert(!xListNode::IsLinked(EventNode));
		EventFlags = IO_EVENT_NONE;
	}
};

class xSocketIoReactor : public xIoReactor {
public:
	X_INLINE xSocket GetNativeSocket() const {
		return NativeSocket;
	}

protected:
	xSocket NativeSocket = InvalidSocket;
};

X_END
