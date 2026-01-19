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

#ifndef NDEBUG
X_PRIVATE void AssertMainThread();
#define XE_ASSERT_MAIN_THREAD() ::xel::AssertMainThread()
#define XE_DEBUG(fmt, ...)      ::xel::XELogger->D(__FILE__, __LINE__, __func__, "" fmt, ##__VA_ARGS__)

#else
#define XE_ASSERT_MAIN_THREAD() ::xel::Pass()
#define XE_DEBUG(...)
#endif

X_END