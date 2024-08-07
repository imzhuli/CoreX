#pragma once
#include "../core/core_min.hpp"
#include "../core/logger.hpp"

X_BEGIN

struct XEngineScopeCallback {
	void (*OnStart)() = nullptr;
	void (*OnStop)()  = nullptr;
};

X_API void RunXEngine(XEngineScopeCallback Callbacks = {});
X_API void StopXEngine();

X_API xLogger * XELogger;  // X_Engine_Loger, only available after init is done.

X_END
