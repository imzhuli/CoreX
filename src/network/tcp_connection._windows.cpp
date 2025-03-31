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
	this->ICP          = IoContextPtr;
	this->LP           = ListenerPtr;
	this->NativeSocket = NativeSocket;
	auto BaseG         = xScopeGuard([this] {
        DestroySocket(std::move(this->NativeSocket));
        xSocketIoReactor::Clean();
        X_DEBUG_RESET(ICP);
        X_DEBUG_RESET(LP);
    });

	// add to event loop
	if (!IoContextPtr->Add(*this)) {
		X_DEBUG_PRINTF("Failed to add connection to IoContext");
		return false;
	}
	auto EG = xScopeGuard([this] { this->ICP->Remove(*this); });

	State = eState::CONNECTED;
	AsyncAcquireInput();

	Dismiss(BaseG, EG);
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
		X_DEBUG_RESET(ICP);
		X_DEBUG_RESET(LP);
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

	State = eState::CONNECTING;
	Dismiss(BaseG, SG, EG);
	return true;
}

void xTcpConnection::Clean() {
	assert(IsOpen());
	this->ICP->Remove(*this);
	DestroySocket(std::move(NativeSocket));
	xSocketIoReactor::Clean();
	State = eState::UNSPEC;
	X_DEBUG_RESET(ICP);
	X_DEBUG_RESET(LP);
}

void xTcpConnection::SuspendReading() {
	ReadingState = eReadingState::SUSPENDED;
}

void xTcpConnection::ResumeReading() {
	if (ReadingState == eReadingState::READING) {
		return;
	}
	ReadingState = eReadingState::READING;
	AsyncAcquireInput();
}

void xTcpConnection::AsyncAcquireInput() {
	if (ReadingState == eReadingState::SUSPENDED) {
		return;
	}
	if (ProcessReading) {  // delay
		return;
	}
	if (AsyncReading) {  // no double entry
		return;
	}
	AsyncReading = true;

	auto StartOffset = IBP->ReadDataSize;
	auto RemainSpace = sizeof(IBP->ReadBuffer) - StartOffset;
	auto TryRecvSize = RemainSpace;
	assert(TryRecvSize);

	auto & BU         = IBP->Reader.BufferUsage;
	BU.buf            = (CHAR *)IBP->ReadBuffer + IBP->ReadDataSize;
	BU.len            = (ULONG)TryRecvSize;
	auto & Overlapped = IBP->Reader.Native.Overlapped;
	memset(&Overlapped, 0, sizeof(Overlapped));
	auto Error = WSARecv(NativeSocket, &BU, 1, nullptr, X2P(DWORD(0)), &Overlapped, nullptr);
	if (Error) {  // clang-format on
		auto ErrorCode = WSAGetLastError();
		if (ErrorCode != WSA_IO_PENDING) {
			X_DEBUG_PRINTF("WSARecv ErrorCode: %u\n", ErrorCode);
			ICP->DeferError(*this);
			return;
		}
	}
	Retain(IBP);
}

void xTcpConnection::AsyncAcquirePost() {
	if (State != eState::CONNECTED || IBP->Writer.AsyncOpMark) {
		return;
	}
	auto WriteBufferPtr = IBP->WriteBufferChain.Pop();
	if (!WriteBufferPtr) {
		return;
	}

	auto & BU = IBP->Writer.BufferUsage;
	BU.buf    = (CHAR *)WriteBufferPtr->Buffer;
	BU.len    = (ULONG)WriteBufferPtr->DataSize;
	memset(&IBP->Writer.Native.Overlapped, 0, sizeof(IBP->Writer.Native.Overlapped));
	auto Error = WSASend(NativeSocket, &BU, 1, nullptr, 0, &IBP->Writer.Native.Overlapped, nullptr);
	if (Error) {
		auto ErrorCode = WSAGetLastError();
		if (ErrorCode != WSA_IO_PENDING) {
			X_DEBUG_PRINTF("ErrorCode: %u\n", ErrorCode);
			ICP->DeferError(*this);
			return;
		}
	}
	IBP->Writer.AsyncOpMark = true;
	Retain(IBP);
	return;
}

void xTcpConnection::PostData(const void * _, size_t DataSize) {
	auto DataPtr = static_cast<const ubyte *>(_);
	if (auto LPBP = IBP->WriteBufferChain.GetLast()) {
		auto PS   = LPBP->Push(DataPtr, DataSize);
		DataPtr  += PS;
		DataSize -= PS;
	}
	while (DataSize) {
		auto BP = new (std::nothrow) xPacketBuffer();
		if (!BP) {
			ICP->DeferError(*this);
			return;
		}
		auto PS   = BP->Push(DataPtr, DataSize);
		DataPtr  += PS;
		DataSize -= PS;
		IBP->WriteBufferChain.Push(BP);
	}
	AsyncAcquirePost();
}

void xTcpConnection::OnIoEventError() {
	LP->OnPeerClose(this);
}

bool xTcpConnection::OnIoEventInReady() {
	AsyncReading = false;
	do {
		auto G = xValueGuard(ProcessReading, true);
		X_DEBUG_PRINTF("ReadDataSize: %zi", IBP->ReadDataSize);
		if (!IBP->ReadDataSize) {
			return false;
		}
		auto ProcessedDataSize = LP->OnData(this, IBP->ReadBuffer, IBP->ReadDataSize);
		if (ProcessedDataSize == InvalidDataSize) {
			return false;
		}
		if ((IBP->ReadDataSize -= ProcessedDataSize)) {
			memmove(IBP->ReadBuffer, IBP->ReadBuffer + ProcessedDataSize, IBP->ReadDataSize);
		}
	} while (false);
	AsyncAcquireInput();
	return true;
}

bool xTcpConnection::OnIoEventOutReady() {
	X_DEBUG_PRINTF("");
	if (State == eState::CONNECTING) {
		State = eState::CONNECTED;
		LP->OnConnected(this);
		AsyncAcquireInput();
	}
	if (auto PostBuffer = (ubyte *)Steal(IBP->Writer.BufferUsage.buf)) {
		auto WBP = X_Entry(PostBuffer, xPacketBuffer, Buffer);
		delete WBP;

		if (Steal(IBP->Writer.LastWriteSize) != Steal(IBP->Writer.BufferUsage.len)) {
			return false;
		}

		assert(IBP->Writer.AsyncOpMark);
		Reset(IBP->Writer.AsyncOpMark);
	}
	AsyncAcquirePost();
	return true;
}

X_END
#endif
