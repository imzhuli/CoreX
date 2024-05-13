#pragma once
#include "./base.hpp"
#include "./net_address.hpp"

X_BEGIN

X_API bool CreateNonBlockingTcpSocket(xSocket & Socket, const xNetAddress & BindAddress = xNetAddress::Make4());
X_API bool CreateNonBlockingUdpSocket(xSocket & Socket, const xNetAddress & BindAddress = xNetAddress::Make4());
X_API void DestroySocket(xSocket && Socket);

X_API void SetSocketNonBlocking(xSocket Socket);
X_API void SetSocketNoDelay(xSocket Socket, bool EnableNoDelay);
X_API void SetSocketReuseAddress(xSocket Socket);

X_API void ResizeSendBuffer(xSocket Socket, size_t Size);
X_API void ResizeRecvBuffer(xSocket Socket, size_t Size);

X_END
