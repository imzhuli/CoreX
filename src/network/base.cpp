#include "./base.hpp"

X_BEGIN

#if defined(X_SYSTEM_WINDOWS)
// const xEventPoller InvalidEventPoller = INVALID_HANDLE_VALUE;

	const xEventPoller InvalidEventPoller = reinterpret_cast<xEventPoller>(INVALID_HANDLE_VALUE);
	const xSocket      InvalidSocket      = INVALID_SOCKET;

    static void InitWinsock() {
        WSADATA WsaData;
        if (WSAStartup(MAKEWORD(2,2), &WsaData)) {
            Fatal("WsaData Error");
        }
    }
    static void CleanupWinsock() {
        WSACleanup();
    }
    static xScopeGuard WSAEnv = { InitWinsock, CleanupWinsock };

#endif

X_END
