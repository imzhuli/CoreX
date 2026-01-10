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

namespace {
	struct xTS : xNonCopyable {
		xTS() {
			FrameSynchronizer.Acquire();
		}
		~xTS() {
			FrameSynchronizer.Release();
		}

		void Sync() { 
			FrameSynchronizer.Synchronize();
		}

		static xThreadSynchronizer FrameSynchronizer;
	};

	xThreadSynchronizer xTS::FrameSynchronizer = {};
}

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

static void FrameLimitThreadFunction() {
	xTimer Timer;
	auto TS = xTS();
	while (EngineRunState) {
		std::this_thread::sleep_for(1ms);
		TS.Sync();
	}
}

static void RenderThreadFunction() {
	size_t FrameCounter = 0;
	xTimer Timer;
	auto TS = xTS();
	while (EngineRunState) {
		++FrameCounter;
		if (Timer.TestAndTag(std::chrono::seconds(1))) {
			std::cout << "FPS: " << FrameCounter << endl;
			FrameCounter = 0;
		}
		xRenderer::UpdateAll();
		TS.Sync();
	}
	xRenderer::CleanAll();
}

static void MainLoop() {
	while (EngineRunState) {
		// no need to call synchronizer here
		WSILoopOnce();
	}
}

void RunXEngine(const XEngineInitOptions & InitOptions) {
	if (!EngineRunState.Start()) {
		cerr << "RunXEngine failed: multiple engine instance" << endl;
		return;
	}
	auto StateCleaner = xScopeGuard([] { EngineRunState.Finish(); });

	if (!InitXEngine()) {
		cerr << "RunXEngine failed: init engine error" << endl;
		return;
	}

	auto OnStart = InitOptions.OnStart;
	auto OnStop = InitOptions.OnStop;

	OnStart();

	auto RenderThread     = std::thread(RenderThreadFunction);
	auto FrameLimitThread = std::thread(FrameLimitThreadFunction);
	MainLoop();
	FrameLimitThread.join();
	RenderThread.join();

	OnStop();

	CleanXEngine();
}

void StopXEngine() {
	EngineRunState.Stop();
}

X_END
