#pragma once
#include "./base.hpp"

X_BEGIN

X_PRIVATE void InitRenderThread();   // run in main thread
X_PRIVATE void CleanRenderThread();  // run in main thread
X_PRIVATE void RenderThreadLoop();

X_PRIVATE xHandle AcquireNoRenderSession();
X_PRIVATE void    ReleaseNoRenderSession(xHandle && NoRenderSessionHandle);

X_END