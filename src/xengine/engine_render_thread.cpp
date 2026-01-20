#include "./engine_render_thread.hpp"

#include "../core/core_time.hpp"
#include "../core/thread.hpp"
#include "./engine.hpp"

#include <atomic>
#include <mutex>
#include <random>

X_BEGIN

static std::mt19937_64 RandomEngine;
static xThreadChecker  RenderThreadChecker;
static xRunState       RenderThreadRunState;

static std::atomic_bool NoRenderSessionRequestLock;
static xAutoResetEvent  NoRenderSessionEnterEvent;
static xAutoResetEvent  NoRenderSessionLeaveEvent;
static uint64_t         NoSessonIdRandom = 0;

static X_VAR xScopeGuard{
	// init
	[] { RandomEngine.seed(GetTimestampUS()); RenderThreadChecker.Init(); },
	// cleanup:
	[] { RenderThreadChecker.Clean(); }
};

xHandle AcquireNoRenderSession() {
	while (!NoRenderSessionRequestLock.compare_exchange_strong(XR(false), true));
	NoRenderSessionEnterEvent.Wait();
	NoSessonIdRandom = RandomEngine() | 0x01;
	return xHandle{ .Native = { .U64 = NoSessonIdRandom } };
}

void ReleaseNoRenderSession(xHandle && NoRenderSessionHandle) {
	assert(NoSessonIdRandom == X_DEBUG_STEAL(NoRenderSessionHandle.Native.U64));
	NoRenderSessionLeaveEvent.Notify();
}

static void LeaveNoRenderSectionCheck() {
	X_DEBUG_PRINTF();
}

static void NoRenderSection() {
	if (!NoRenderSessionRequestLock.compare_exchange_strong(XR(true), false)) {
		return;
	}
	NoRenderSessionEnterEvent.Notify();
	NoRenderSessionLeaveEvent.Wait(LeaveNoRenderSectionCheck);
}

void InitRenderThread() {
	XE_ASSERT_MAIN_THREAD();
	RuntimeAssert(RenderThreadRunState.Start());
}

void CleanRenderThread() {
	XE_ASSERT_MAIN_THREAD();
	RenderThreadRunState.Finish();
}

void StopRenderThreadLoop() {
	RenderThreadRunState.Stop();
}

void RenderThreadLoop() {
	// loop
	while (RenderThreadRunState) {
		// FreeRender options
		X_DEBUG_PRINTF();
		NoRenderSection();
	}
}

X_END
