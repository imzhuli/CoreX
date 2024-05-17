#include "./tcp_connection.hpp"

#ifdef X_SYSTEM_WINDOWS

#include "../core/string.hpp"
#include "./socket.hpp"

#include <atomic>
#include <mutex>

X_BEGIN

std::atomic<LPFN_CONNECTEX> AtomicConnectEx = nullptr;
std::mutex                  ConnectExLoaderMutex;

bool xTcpConnection::Init(xIoContext * IoContextPtr, xSocket && NativeSocket, iListener * ListenerPtr) {
	if (!xSocketIoReactor::Init()) {
		return false;
	}
	this->ICP  = IoContextPtr;
	this->LP   = ListenerPtr;
	auto BaseG = xScopeGuard([this] {
		xSocketIoReactor::Clean();
		Reset(ICP);
		Reset(LP);
	});

	Todo("");

	Dismiss(BaseG);
	return true;
}

bool xTcpConnection::Init(xIoContext * IoContextPtr, const xNetAddress & TargetAddress, const xNetAddress & BindAddress, iListener * ListenerPtr) {
	auto AddressType = BindAddress.Type;
	assert(AddressType && AddressType == TargetAddress.Type);
	if (!xSocketIoReactor::Init()) {
		return false;
	}
	this->ICP  = IoContextPtr;
	this->LP   = ListenerPtr;
	auto BaseG = xScopeGuard([this] {
		xSocketIoReactor::Clean();
		Reset(ICP);
		Reset(LP);
	});

	if (!CreateNonBlockingTcpSocket(NativeSocket, BindAddress)) {
		// X_PERROR("Failed to create non blocking tcp socket");
		return false;
	}
	auto SG = xScopeGuard([this] { DestroySocket(std::move(NativeSocket)); });

	// add to event loop
	if (!IoContextPtr->Add(*this)) {
		X_DEBUG_PRINTF("Failed to add connection to IoContext");
		return false;
	}
	auto EG = xScopeGuard([this] { this->ICP->Remove(*this); });

	do {  // async connect
		LPFN_CONNECTEX ConnectEx = AtomicConnectEx.load();
		// load ConnectEx for async connect
		if (!ConnectEx) {
			do {
				auto LockGuard = std::lock_guard(ConnectExLoaderMutex);
				if (ConnectEx = AtomicConnectEx.load()) {
					break;
				}
				GUID  guid      = WSAID_CONNECTEX;
				DWORD dwBytes   = 0;
				auto  LoadError = WSAIoctl(NativeSocket, SIO_GET_EXTENSION_FUNCTION_POINTER, &guid, sizeof(guid), &ConnectEx, sizeof(ConnectEx), &dwBytes, NULL, NULL);
				if (LoadError) {
					auto ErrorCode = WSAGetLastError();
					Touch(ErrorCode);
					X_DEBUG_PRINTF("ErrorCode: %u\n", ErrorCode);
					return false;
				}
				X_DEBUG_PRINTF("ConnectEx: %p\n", ConnectEx);
				AtomicConnectEx = ConnectEx;
			} while (false);
		}
		sockaddr_storage AddrStorage = {};
		size_t           AddrLen     = TargetAddress.Dump(&AddrStorage);
		auto             Success     = ConnectEx(NativeSocket, (SOCKADDR *)(&AddrStorage), (int)AddrLen, NULL, NULL, NULL, &IBP->Writer.Native.Overlapped);
		if (!Success) {
			auto ErrorCode = WSAGetLastError();
			if (ErrorCode != ERROR_IO_PENDING) {
				X_DEBUG_PRINTF("Failed to build connection ErrorCode: %u (ERROR_IO_PENDING == 997L)\n", ErrorCode);
				return false;
			} else {
				X_DEBUG_PRINTF("PENDING");
			}
		}
		Retain(IBP);
	} while (false);
	AsyncAcquireInput();

	State = eState::CONNECTING;
	Dismiss(BaseG, SG, EG);
	return true;
}

void xTcpConnection::Clean() {
	this->ICP->Remove(*this);
	DestroySocket(std::move(NativeSocket));
	xSocketIoReactor::Clean();
	Reset(ICP);
	Reset(LP);
}

xNetAddress xTcpConnection::GetRemoteAddress() const {
	Todo("");
}

xNetAddress xTcpConnection::GetLocalAddress() const {
	Todo("");
}

void xTcpConnection::AsyncAcquireInput() {
	auto StartOffset = IBP->ReadDataSize;
	auto RemainSpace = sizeof(IBP->ReadBuffer) - StartOffset;
	auto TryRecvSize = std::min(100, RemainSize);  // for debug

	Todo("");

	IBP->ReadDataSize = 0;
	memset(&IBP->Reader.Native.Overlapped, 0, sizeof(IBP->Reader.Native.Overlapped));
	memset(&IBP->Reader.FromAddress, 0, sizeof(IBP->Reader.FromAddress));
	IBP->Reader.FromAddressLength = sizeof(IBP->Reader.FromAddress);
	IBP->Reader.BufferUsage.buf   = (CHAR *)IBP->ReadBuffer;
	IBP->Reader.BufferUsage.len   = (ULONG)sizeof(IBP->ReadBuffer);
	if (WSARecvFrom(  // clang-format off
			NativeSocket, &IBP->Reader.BufferUsage, 1, nullptr, X2P(DWORD(0)),
			(sockaddr *)&IBP->Reader.FromAddress, &IBP->Reader.FromAddressLength,
			&IBP->Reader.Native.Overlapped,
			nullptr
		)) {  // clang-format on
		auto ErrorCode = WSAGetLastError();
		if (ErrorCode != WSA_IO_PENDING) {
			X_DEBUG_PRINTF("WSARecvFrom ErrorCode: %u\n", ErrorCode);
			ICP->DeferError(*this);
			return;
		}
	}
	Retain(IBP);
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
	Todo("");
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
	X_DEBUG_PRINTF("");
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
