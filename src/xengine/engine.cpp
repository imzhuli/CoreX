#include "./engine.hpp"

#include "../core/core_time.hpp"
#include "../core/thread.hpp"
#include "../vk/vk.hpp"
#include "../wsi/wsi.hpp"
#include "./engine_main_ui_thread.hpp"
#include "./engine_render_thread.hpp"

#include <curl/curl.h>

#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

using std::cerr;
using std::endl;
using namespace std::chrono_literals;

X_BEGIN

static xStdLogger EngineLogger;

xLogger *           XELogger = &EngineLogger;
xRunState           XERunState;
xThreadChecker      XEMainThreadChecker;
xThreadSynchronizer XECleanupSynchronizer;

static bool InitXEngine() {
	if (!InitWSI()) {
		cerr << "Failed to init wsi" << endl;
		return false;
	}
	auto WSICleaner = xScopeGuard(CleanWSI);

	if (!InitVulkan()) {
		cerr << "Failed to init vulkan" << endl;
		return false;
	}
	auto VkCleaner = xScopeGuard(CleanVulkan);

	WSICleaner.Dismiss();
	VkCleaner.Dismiss();
	return true;
}

static void CleanXEngine() {
	CleanVulkan();
	CleanWSI();
	return;
}

void AssertMainThread() {
	XEMainThreadChecker.Assert();
}

void RunXEngine(const xEngineInitOptions & InitOptions) {
	if (!XERunState.Start()) {
		X_PFATAL("RunXEngine failed: multiple engine instance");
		return;
	}
	X_SCOPE_GUARD([] { XERunState.Finish(); });

	if (!InitXEngine()) {
		X_PFATAL("RunXEngine failed: init engine error");
		return;
	}
	X_SCOPE_GUARD(CleanXEngine);

	X_RESOURCE_GUARD(XEMainThreadChecker);
	do {
		auto OnStart = InitOptions.OnStart;
		auto OnStop  = InitOptions.OnStop;

		// don't reorder the following:
		X_SCOPE_GUARD(InitRenderThread, CleanRenderThread);
		X_SCOPE_GUARD(InitMainUIThread, CleanMainUIThread);

		auto RenderThread = std::thread(RenderThreadLoop);

		OnStart();
		MainUIThreadLoop();
		OnStop();

		RenderThread.join();
	} while (false);
}

void StopXEngine() {
	XERunState.Stop();
}

X_END
