#pragma once
#include "./base.hpp"

X_BEGIN

using xNoRenderSession = uint64_t;

X_PRIVATE void InitRenderThread();   // run in main thread
X_PRIVATE void CleanRenderThread();  // run in main thread
X_PRIVATE void RenderThreadLoop();
X_PRIVATE void StopRenderThreadLoop();

X_PRIVATE xNoRenderSession AcquireNoRenderSession();
X_PRIVATE void             ReleaseNoRenderSession(xNoRenderSession && NoRenderSessionHandle);

X_END