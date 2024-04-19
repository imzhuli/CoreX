#include "./tcp_connection.hpp"

#include "../core/string.hpp"
#include "./socket.hpp"

#if defined(X_SYSTEM_DARWIN) || defined(X_SYSTEM_LINUX)
X_BEGIN

bool xTcpConnection::Init(xIoContext * IoContextPtr, xSocket && NativeSocket, iListener * ListenerPtr) {
	this->ICP = IoContextPtr;
	this->LP  = ListenerPtr;
	if (!xSocketIoReactor::Init()) {
		return false;
	}
	auto BaseG = xScopeGuard([this] {
		xSocketIoReactor::Clean();
		Reset(ICP);
		Reset(LP);
	});

	this->NativeSocket = NativeSocket;
	SetSocketNonBlocking(NativeSocket);
	auto SG = xScopeGuard([this] { DestroySocket(std::move(this->NativeSocket)); });

	if (!IoContextPtr->Add(*this)) {
		return false;
	}

	State = eState::CONNECTED;
	IoContextPtr->DeferWrite(*this);

	Dismiss(BaseG, SG);
	return true;
}

bool xTcpConnection::Init(xIoContext * IoContextPtr, const xNetAddress & TargetAddress, const xNetAddress & BindAddress, iListener * ListenerPtr) {
	assert(TargetAddress.Type == BindAddress.Type);
	this->ICP = IoContextPtr;
	this->LP  = ListenerPtr;
	if (!xSocketIoReactor::Init()) {
		return false;
	}
	auto BaseG = xScopeGuard([this] {
		xSocketIoReactor::Clean();
		Reset(ICP);
		Reset(LP);
	});

	if (!CreateNonBlockingTcpSocket(NativeSocket, BindAddress)) {
		return false;
	}
	auto SG = xScopeGuard([this] { DestroySocket(std::move(NativeSocket)); });

	auto Connected = false;
	auto SA        = sockaddr_storage();
	auto SL        = TargetAddress.Dump(&SA);
	auto CR        = connect(NativeSocket, (sockaddr *)&SA, (socklen_t)SL);
	if (0 == CR) {  // connected:
		Connected = true;
	} else {
		auto EN = errno;
		if (EN != EINPROGRESS) {
			return false;
		}
	}

	if (!IoContextPtr->Add(*this, true, !Connected)) {
		return false;
	}
	State = eState::CONNECTING;
	if (Connected) {
		IoContextPtr->DeferWrite(*this);
	}

	Dismiss(BaseG, SG);
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
	sockaddr_storage SockAddr;
	socklen_t        SockAddrLen = sizeof(SockAddr);
	if (getpeername(NativeSocket, (sockaddr *)&SockAddr, &SockAddrLen)) {
		return {};
	}
	return xNetAddress::Parse(&SockAddr);
}

xNetAddress xTcpConnection::GetLocalAddress() const {
	sockaddr_storage SockAddr;
	socklen_t        SockAddrLen = sizeof(SockAddr);
	if (getsockname(NativeSocket, (sockaddr *)&SockAddr, &SockAddrLen)) {
		return {};
	}
	return xNetAddress::Parse(&SockAddr);
}

bool xTcpConnection::ReadData(xView<ubyte> & BufferView) {
	Reset(BufferView);
	auto & ReadBuffer   = IBP->ReadBuffer;
	auto & ReadDataSize = IBP->ReadDataSize;

	auto SpaceSize = sizeof(ReadBuffer) - ReadDataSize;
	if (!SpaceSize) {
		X_DEBUG_PRINTF("too many unprocessed bytes, treated as error");
		return false;
	}
	auto StartPtr = ReadBuffer + ReadDataSize;
	auto ReadSize = read(NativeSocket, StartPtr, SpaceSize);
	if (0 == ReadSize) {
		return false;  // peer close
	}
	if (-1 == ReadSize) {
		auto Error = errno;
		if (EAGAIN == Error) {
			return true;
		}
		return false;
	}
	ReadDataSize += ReadSize;
	BufferView    = { ReadBuffer, (size_t)ReadDataSize };
	return true;
}

void xTcpConnection::PostData(const void * _, size_t DataSize) {
	assert(DataSize);
	auto   DataPtr          = static_cast<const ubyte *>(_);
	auto & WriteBufferChain = IBP->WriteBufferChain;

	auto HasNoPendingWrite = WriteBufferChain.IsEmpty();
	if (HasNoPendingWrite) {
		ssize_t FirstSend = send(NativeSocket, DataPtr, DataSize, XelNoWriteSignal);
		if (-1 == FirstSend) {
			if (errno == EAGAIN) {
				goto BUFFER_WRITE;
			}
			ICP->DeferError(*this);
			return;
		}
		assert(FirstSend >= 0);
		if (!(DataSize -= FirstSend)) {  // all sent
			return;
		}
		DataPtr += FirstSend;
	} else {  // try push to last first
		auto LPBP = WriteBufferChain.GetLast();
		auto PS   = LPBP->Push(DataPtr, DataSize);
		DataPtr  += PS;
		DataSize -= PS;
	}
BUFFER_WRITE:
	while (DataSize) {
		auto BP = new (std::nothrow) xPacketBuffer();
		if (!BP) {
			ICP->DeferError(*this);
			return;
		}
		auto PS   = BP->Push(DataPtr, DataSize);
		DataPtr  += PS;
		DataSize -= PS;
		WriteBufferChain.Push(BP);
	}
	if (HasNoPendingWrite) {
		ICP->Update(*this, true, true);
	}
	return;
}

void xTcpConnection::OnIoEventError() {
	LP->OnPeerClose(this);
}

bool xTcpConnection::OnIoEventInReady() {
	auto & ReadBuffer   = IBP->ReadBuffer;
	auto & ReadDataSize = IBP->ReadDataSize;

	auto NewInput = xView<ubyte>();
	while (true) {
		if (!ReadData(NewInput)) {
			return false;
		}
		if (!NewInput) {
			break;
		}
		auto ProcessedDataSize = LP->OnData(this, NewInput.Data(), NewInput.Size());
		if (ProcessedDataSize == InvalidDataSize) {
			return false;
		}
		if ((ReadDataSize -= ProcessedDataSize)) {
			memmove(ReadBuffer, ReadBuffer + ProcessedDataSize, ReadDataSize);
		}
	}
	return true;
}

bool xTcpConnection::OnIoEventOutReady() {
	if (State == eState::CONNECTING) {
		State = eState::CONNECTED;
		LP->OnConnected(this);
		return true;
	}
	auto & WriteBufferChain = IBP->WriteBufferChain;
	while (auto BP = WriteBufferChain.Peek()) {
		auto WS = send(NativeSocket, BP->Buffer, BP->DataSize, XelNoWriteSignal);
		if (-1 == WS) {
			return (EAGAIN == errno);
		}
		if (!(BP->DataSize -= WS)) {
			WriteBufferChain.RemoveFront();
			delete BP;
			continue;
		}
		memmove(BP->Buffer, BP->Buffer + WS, BP->DataSize);
		return true;
	}
	ICP->Update(*this);
	return true;
}

X_END
#endif
