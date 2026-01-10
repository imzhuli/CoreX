#pragma once
#include "../core/core_min.hpp"
#include "../core/functional.hpp"
#include "../core/logger.hpp"

X_BEGIN

struct XEngineInitOptions {
	std::function<void()> OnStart = Noop<>;
	std::function<void()> OnStop  = Noop<>;
};

X_API void RunXEngine(const XEngineInitOptions & InitOptions = {});
X_API void StopXEngine();

X_API xLogger * XELogger;  // First object inited only available after init is done.

X_END
