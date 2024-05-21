#pragma once
#include "../core/core_min.hpp"
#include "../core/core_value_util.hpp"
#include "../core/list.hpp"
#include "./base.hpp"
#include "./packet.hpp"
#include "./packet_buffer.hpp"

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
#ifdef X_SYSTEM_WINDOWS
	virtual void SetReadTransfered(size_t Size)  = 0;
	virtual void SetWriteTransfered(size_t Size) = 0;
#endif
};

struct xIoBuffer {
	ubyte              ReadBuffer[MaxPacketSize];
	size_t             ReadDataSize = 0;
	xPacketBufferChain WriteBufferChain;
	size_t             MaxWriteBufferSize = SIZE_MAX / 2;
#if defined(X_SYSTEM_DARWIN) || defined(X_SYSTEM_LINUX)  // clang-format off
	X_INLINE xIoBuffer * operator->() { return this; }
	X_INLINE const xIoBuffer * operator->() const { return this; }
#endif  // clang-format on
};

#if defined(X_SYSTEM_WINDOWS)

struct xOverlappedObject;
struct xOverlappedIoBuffer;
using xIOBCleaner = void(xOverlappedIoBuffer *);

struct xOverlappedObject : private xNonCopyable {
	xOverlappedIoBuffer * Outter;
	OVERLAPPED            Overlapped;
};

struct xOverlappedIoBuffer : xIoBuffer {
	xIoReactor * Reactor        = nullptr;
	ssize_t      ReferenceCount = 1;
	bool         Enabled        = false;  // with reactor closed, multiple events might still be active, RefCount is not enough.
	struct {
		xOverlappedObject Native;
		sockaddr_storage  FromAddress;
		int               FromAddressLength;
		WSABUF            BufferUsage;  // reused as tcp server PreAcceptPointer
	} Reader = {};
	struct {
		xOverlappedObject Native;
		WSABUF            BufferUsage;
		size_t            LastWriteSize;
		bool              AsyncOpMark;
	} Writer              = {};
	xIOBCleaner * Cleaner = nullptr;
};
X_API void                  Retain(xOverlappedIoBuffer * IBP);
X_API xOverlappedIoBuffer * Release(xOverlappedIoBuffer * IBP);  // null: object deleted

#endif  // X_SYSTEM_WINDOWS

class xSocketIoReactor
	: public xIoReactor
	, xNonCopyable {
public:
	X_API_MEMBER bool Init();
	X_API_MEMBER void Clean();

	X_INLINE xSocket GetNativeSocket() const {
		return NativeSocket;
	}

protected:
	xSocket NativeSocket = InvalidSocket;

#if defined(X_SYSTEM_DARWIN) || defined(X_SYSTEM_LINUX)
	xIoBuffer IBP;
#elif defined(X_SYSTEM_WINDOWS)
	friend class xIoContext;
	xOverlappedIoBuffer * IBP = nullptr;

	void SetReadTransfered(size_t Size) override {
		if (!Size) {
			IBP->ReadDataSize = 0;
			return;
		}
		IBP->ReadDataSize += Size;
	}
	void SetWriteTransfered(size_t Size) override {
		IBP->Writer.LastWriteSize = Size;
	}
#endif
};

X_END
