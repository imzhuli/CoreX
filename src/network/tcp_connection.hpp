#pragma once
#include "../core/view.hpp"
#include "./io_context.hpp"
#include "./packet.hpp"
#include "./packet_buffer.hpp"
#include "./socket.hpp"

X_BEGIN

class xTcpConnection
	: xSocketIoReactor
	, xAbstract {
public:
	enum struct eState : uint16_t {
		UNSPEC,
		CONNECTING,
		CONNECTED,
	};

	enum struct eReadingState : uint16_t {
		READING,
		SUSPENDED,
	};

	struct iListener {
		// callback on connected, normally this is not needed to be handled
		X_API_MEMBER virtual void   OnConnected(xTcpConnection * TcpConnectionPtr) {}
		X_API_MEMBER virtual void   OnPeerClose(xTcpConnection * TcpConnectionPtr) {}
		X_API_MEMBER virtual void   OnFlush(xTcpConnection * TcpConnectionPtr) {}
		X_API_MEMBER virtual size_t OnData(xTcpConnection * TcpConnectionPtr, ubyte * DataPtr, size_t DataSize) { return DataSize; }
	};

public:
	X_API_MEMBER bool Init(xIoContext * IoContextPtr, xSocket && NativeSocket, iListener * ListenerPtr);
	X_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & TargetAddress, const xNetAddress & BindAddress, iListener * ListenerPtr);
	X_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & TargetAddress, iListener * ListenerPtr) {
		assert(TargetAddress);
		return Init(IoContextPtr, TargetAddress, TargetAddress.Is4() ? xNetAddress::Make4() : xNetAddress::Make6(), ListenerPtr);
	}
	X_API_MEMBER void Clean();

	X_API_MEMBER xNetAddress GetRemoteAddress() const;
	X_API_MEMBER xNetAddress GetLocalAddress() const;

	X_INLINE bool     IsOpen() const { return State != eState::UNSPEC; }
	X_INLINE bool     IsConnected() const { return State == eState::CONNECTED; }
	X_INLINE void     ResizeSendBuffer(size_t Size) { xel::ResizeSendBuffer(NativeSocket, Size); };
	X_INLINE void     ResizeRecvBuffer(size_t Size) { xel::ResizeRecvBuffer(NativeSocket, Size); };
	X_INLINE void     SetMaxWriteBufferSize(size_t NewLimit) { IBP->MaxWriteBufferSize = NewLimit; }
	X_INLINE bool     HasPendingWrites() const { return IBP->WriteBufferChain.GetSize(); }
	X_API_MEMBER void PostData(const void * DataPtr, size_t DataSize);
	X_API_MEMBER void PostRequestKeepAlive();
	X_API_MEMBER void PostKeepAlive();
	X_API_MEMBER void SuspendReading();
	X_API_MEMBER void ResumeReading();

protected:
	X_API_MEMBER void OnIoEventError() override;
	X_API_MEMBER bool OnIoEventInReady() override;
	X_API_MEMBER bool OnIoEventOutReady() override;

#ifdef X_SYSTEM_WINDOWS
	X_API_MEMBER void AsyncAcquireInput();
	X_API_MEMBER void AsyncAcquirePost();
#else
	X_API_MEMBER bool ReadData(xView<ubyte> & BufferView);
#endif

private:
	xIoContext *  ICP          = nullptr;
	iListener *   LP           = nullptr;
	eState        State        = eState::UNSPEC;
	eReadingState ReadingState = eReadingState::READING;

#ifdef X_SYSTEM_WINDOWS
	struct {
		uint8_t ProcessReading = false;
		uint8_t AsyncReading   = false;
	};
#endif
};

X_END
