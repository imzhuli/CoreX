#include "./tcp_connection.hpp"

#include "../core/string.hpp"
#include "./socket.hpp"

#ifdef X_SYSTEM_WINDOWS
X_BEGIN

bool xTcpConnection::Init(xIoContext * IoContextPtr, xSocket && NativeSocket, iListener * ListenerPtr) {
	if (!xSocketIoReactor::Init()) {
		return false;
	}
	auto BaseG = xScopeGuard([this] { xSocketIoReactor::Clean(); });

	Todo();

	Dismiss(BaseG);
	return true;
}

bool xTcpConnection::Init(xIoContext * IoContextPtr, const xNetAddress & TargetAddress, const xNetAddress & BindAddress, iListener * ListenerPtr) {
	assert(TargetAddress.Type == BindAddress.Type);
	if (!xSocketIoReactor::Init()) {
		return false;
	}
	auto BaseG = xScopeGuard([this] { xSocketIoReactor::Clean(); });

	Todo();

	Dismiss(BaseG);
	return true;
}

void xTcpConnection::Clean() {
	Todo();
	xSocketIoReactor::Clean();
}

xNetAddress xTcpConnection::GetRemoteAddress() const {
	Todo();
}

xNetAddress xTcpConnection::GetLocalAddress() const {
	Todo();
}

bool xTcpConnection::ReadData(xView<ubyte> & BufferView) {
	return false;
}

void xTcpConnection::PostData(const void * _, size_t DataSize) {
}

void xTcpConnection::OnIoEventError() {
	LP->OnPeerClose(this);
}

bool xTcpConnection::OnIoEventInReady() {
	Todo();
	return false;
	// auto NewInput = xView<ubyte>();
	// while (true) {
	// 	if (!ReadData(NewInput)) {
	// 		return false;
	// 	}
	// 	if (!NewInput) {
	// 		break;
	// 	}
	// 	auto ProcessedDataSize = LP->OnData(this, NewInput.Data(), NewInput.Size());
	// 	if (ProcessedDataSize == InvalidDataSize) {
	// 		return false;
	// 	}
	// 	if ((ReadDataSize -= ProcessedDataSize)) {
	// 		memmove(ReadBuffer, ReadBuffer + ProcessedDataSize, ReadDataSize);
	// 	}
	// }
	// return true;
}

bool xTcpConnection::OnIoEventOutReady() {
	Todo();
	return false;
	// if (State == eState::CONNECTING) {
	// 	State = eState::CONNECTED;
	// 	LP->OnConnected(this);
	// 	return true;
	// }
	// while (auto BP = WriteBufferChain.Peek()) {
	// 	auto WS = send(NativeSocket, BP->Buffer, BP->DataSize, XelNoWriteSignal);
	// 	if (-1 == WS) {
	// 		return (EAGAIN == errno);
	// 	}
	// 	if (!(BP->DataSize -= WS)) {
	// 		WriteBufferChain.RemoveFront();
	// 		delete BP;
	// 		continue;
	// 	}
	// 	memmove(BP->Buffer, BP->Buffer + WS, BP->DataSize);
	// 	return true;
	// }
	// ICP->Update(*this);
	// return true;
}

X_END
#endif
