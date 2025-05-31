// clang-format off
#pragma once

#include "../core/core_min.hpp"

#if defined(X_SYSTEM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
/*
!!! DO NOT reorder include files
 */
#include <WinSock2.h>
#include <Windows.h>
#include <MSWSock.h>
#include <WS2tcpip.h>
#include <windef.h>
#include <ws2def.h>

X_BEGIN
typedef SSIZE_T          ssize_t;
typedef int              send_len_t;
typedef int              recv_len_t;
typedef HANDLE           xEventPoller;
typedef xVariable        xNativeEventType;
typedef SOCKET           xSocket;
X_API const xEventPoller InvalidEventPoller;
X_API const xSocket      InvalidSocket;
#define XelCloseSocket(sockfd) closesocket((sockfd))
X_END

#elif defined(X_SYSTEM_LINUX)
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

X_BEGIN
typedef size_t send_len_t;
typedef size_t recv_len_t;
typedef int    xEventPoller;  // epoll
#ifdef X_SYSTEM_ANDROID
typedef uint32_t xNativeEventType;
#else
typedef enum EPOLL_EVENTS xNativeEventType;  // EPOLLIN EPOLLOUT EPOLLERR ...
#endif
typedef int                  xSocket;
constexpr const xEventPoller InvalidEventPoller = ((xEventPoller)-1);
constexpr const xSocket      InvalidSocket      = ((xSocket)-1);
#define XelCloseSocket(sockfd) close((sockfd))
X_END

#elif defined(X_SYSTEM_DARWIN)
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

X_BEGIN

typedef size_t               send_len_t;
typedef size_t               recv_len_t;
typedef int                  xEventPoller;  // kqueue
typedef int                  xSocket;
constexpr const xEventPoller InvalidEventPoller = ((xEventPoller)-1);
constexpr const xSocket      InvalidSocket      = ((xSocket)-1);
#define XelCloseSocket(sockfd) close((sockfd))

X_END

#else
#error unsupported system type
#endif

#ifdef MSG_NOSIGNAL
#define XelNoWriteSignal MSG_NOSIGNAL
#else
#define XelNoWriteSignal 0
#endif
#ifndef SOCK_CLOEXEC
#define SOCK_CLOEXEC 0
#endif
