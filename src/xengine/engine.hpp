#pragma once
#include "../core/functional.hpp"
#include "./base.hpp"
#include "./engine_config.hpp"

X_BEGIN

struct xEngineInitOptions {
	std::function<void()> OnStart = Noop<>;
	std::function<void()> OnStop  = Noop<>;
};

X_API void RunXEngine(const xEngineInitOptions & InitOptions = {});
X_API void StopXEngine();

X_END

#ifndef NDEBUG
X_PRIVATE void AssertMainThread();
#define XE_ASSERT_MAIN_THREAD() AssertMainThread()
#define XE_DEBUG(fmt, ...)      ::xel::XELogger->D(__FILE__, __LINE__, __func__, "" fmt, ##__VA_ARGS__)

#else
#define XE_ASSERT_MAIN_THREAD()
#define XE_DEBUG(...)
#endif
