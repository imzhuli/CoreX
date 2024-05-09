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
	enum struct eState {
		UNSPEC,
		CONNECTING,
		CONNECTED,
		CLOSING,
	};

	struct iListener {  // clang-format off
		// callback on connected, normally this is not needed to be handled
		X_API_MEMBER virtual void OnConnected(xTcpConnection * TcpConnectionPtr) {}
		X_API_MEMBER virtual void OnPeerClose(xTcpConnection * TcpConnectionPtr) {}
		X_API_MEMBER virtual void OnFlush(xTcpConnection * TcpConnectionPtr) {}
		X_API_MEMBER virtual size_t OnData(xTcpConnection * TcpConnectionPtr, void * DataPtr, size_t DataSize) { return DataSize; }
	};  // clang-format on

public:
	X_API_MEMBER bool Init(xIoContext * IoContextPtr, xSocket && NativeSocket, iListener * ListenerPtr);
	X_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & TargetAddress, const xNetAddress & BindAddress, iListener * ListenerPtr);
	X_API_MEMBER bool Init(xIoContext * IoContextPtr, const xNetAddress & TargetAddress, iListener * ListenerPtr) {
		assert(TargetAddress);
		return Init(IoContextPtr, TargetAddress, TargetAddress.IsV4() ? xNetAddress::Make4() : xNetAddress::Make6(), ListenerPtr);
	}
	X_API_MEMBER void Clean();

	X_API_MEMBER xNetAddress GetRemoteAddress() const;
	X_API_MEMBER xNetAddress GetLocalAddress() const;

	// clang-format off
	X_INLINE bool IsConnected() const { return State == eState::CONNECTED; }
	X_INLINE void SetMaxWriteBufferSize(size_t NewLimit) { IBP->MaxWriteBufferSize = NewLimit; }
	X_INLINE bool HasPendingWrites() const { return IBP->WriteBufferChain.GetSize(); }
	X_API_MEMBER void PostData(const void * DataPtr, size_t DataSize);
	X_API_MEMBER void PostRequestKeepAlive();
	X_API_MEMBER void PostKeepAlive();
	// clang-format on
protected:
	X_API_MEMBER void OnIoEventError() override;
	X_API_MEMBER bool OnIoEventInReady() override;
	X_API_MEMBER bool OnIoEventOutReady() override;

	X_API_MEMBER bool ReadData(xView<ubyte> & BufferView);

private:
	xIoContext * ICP   = nullptr;
	iListener *  LP    = nullptr;
	eState       State = eState::UNSPEC;
};

X_END
