#include "./engine.hpp"

#include "../core/core_time.hpp"
#include "../core/thread.hpp"
#include "../renderer/renderer.hpp"
#include "../vk/vk.hpp"
#include "../wsi/wsi.hpp"

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
xLogger *         XELogger = &EngineLogger;

static xRunState           EngineRunState;
static xThreadSynchronizer FrameSynchronizer;

bool InitXEngine() {
	if (!InitWSI()) {
		cerr << "Failed to init wsi" << endl;
		return false;
	}
	auto WSICleaner = xScopeGuard([] { CleanWSI(); });

	if (!InitVulkan()) {
		cerr << "Failed to init vulkan" << endl;
		return false;
	}
	auto VkCleaner = xScopeGuard([] { CleanVulkan(); });

	if (curl_global_init(CURL_GLOBAL_ALL)) {
		cerr << "Failed to init curl" << endl;
		return false;
	}
	auto CurlCleaner = xScopeGuard([] { curl_global_cleanup(); });

	WSICleaner.Dismiss();
	VkCleaner.Dismiss();
	CurlCleaner.Dismiss();
	return true;
}

void CleanXEngine() {
	curl_global_cleanup();
	CleanVulkan();
	CleanWSI();
	return;
}

void FrameLimitThreadFunction() {
	xTimer Timer;
	FrameSynchronizer.Acquire();
	while (EngineRunState) {
		std::this_thread::sleep_for(1ms);
		FrameSynchronizer.Synchronize();
	}
	FrameSynchronizer.Release();
}

void RenderThreadFunction() {
	size_t FrameCounter = 0;
	xTimer Timer;
	FrameSynchronizer.Acquire();
	while (EngineRunState) {
		++FrameCounter;
		if (Timer.TestAndTag(std::chrono::seconds(1))) {
			std::cout << "FPS: " << FrameCounter << endl;
			FrameCounter = 0;
		}
		xRenderer::UpdateAll();
		FrameSynchronizer.Synchronize();
	}
	FrameSynchronizer.Release();
	xRenderer::CleanAll();
}

void MainLoop() {
	while (EngineRunState) {
		// no need to call synchronizer here
		WSILoopOnce();
	}
}

void RunXEngine(XEngineScopeCallback Callbacks) {
	if (!EngineRunState.Start()) {
		cerr << "RunXEngine failed: multiple engine instance" << endl;
		return;
	}
	auto StateCleaner = xScopeGuard([] { EngineRunState.Finish(); });

	if (!InitXEngine()) {
		cerr << "RunXEngine failed: init engine error" << endl;
		return;
	}

	if (Callbacks.OnStart) {
		Callbacks.OnStart();
	}

	auto RenderThread     = std::thread(RenderThreadFunction);
	auto FrameLimitThread = std::thread(FrameLimitThreadFunction);
	MainLoop();
	FrameLimitThread.join();
	RenderThread.join();

	if (Callbacks.OnStop) {
		Callbacks.OnStop();
	}
	CleanXEngine();
}

void StopXEngine() {
	EngineRunState.Stop();
}

X_END
