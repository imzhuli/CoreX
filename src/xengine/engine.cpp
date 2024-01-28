#include "./engine.hpp"

#include "../core/core_chrono.hpp"
#include "../core/thread.hpp"
#include "../renderer/renderer.hpp"
#include "../vk/vk.hpp"
#include "../wsi/wsi.hpp"

#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <thread>

using std::cerr;
using std::endl;
using namespace std::chrono_literals;

X_BEGIN

static xBaseLogger EngineLogger;
xLogger *          Logger = &EngineLogger;

static constexpr const uint8_t EngineState_NoInstance = 0;
static constexpr const uint8_t EngineState_Running    = 0x01;
static constexpr const uint8_t EngineState_Stopping   = 0x02;

static std::atomic_uint8_t EngineInstanceState;
static xThreadSynchronizer FrameSynchronizer;

bool InitXEngine() {
	if (!EngineLogger.Init()) {
		cerr << "Failed to init logger" << endl;
		return false;
	}
	auto LogCleaner = MakeResourceCleaner(EngineLogger);

	if (!InitWSI()) {
		return false;
	}
	auto WSICleaner = xScopeGuard([] { CleanWSI(); });

	if (!InitVulkan()) {
		return false;
	}

	WSICleaner.Dismiss();
	LogCleaner.Dismiss();
	return true;
}

void CleanXEngine() {
	CleanVulkan();
	CleanWSI();
	EngineLogger.Clean();
	return;
}

void FrameLimitThreadFunction() {
	xTimer Timer;
	FrameSynchronizer.Acquire();
	while (EngineInstanceState.load() == EngineState_Running) {
		std::this_thread::sleep_for(9ms);
		FrameSynchronizer.Synchronize();
	}
	FrameSynchronizer.Release();
}

void RenderThreadFunction() {
	size_t FrameCounter = 0;
	xTimer Timer;
	FrameSynchronizer.Acquire();
	while (EngineInstanceState.load() == EngineState_Running) {
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
	while (EngineInstanceState.load() == EngineState_Running) {
		// Note! dont use synchronizer here, since rendering thread may have implicit lock with the wsi message event.
		WSILoopOnce();
	}
}

void RunXEngine(XEngineScopeCallback Callbacks) {
	auto Expected    = EngineState_NoInstance;
	bool NewInstance = EngineInstanceState.compare_exchange_strong(Expected, EngineState_Running);
	if (!NewInstance) {
		cerr << "RunXEngine failed: multiple engine instance" << endl;
		return;
	}

	auto StateCleaner = xScopeGuard([] { EngineInstanceState = EngineState_NoInstance; });
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
	auto Expected = EngineState_Running;
	EngineInstanceState.compare_exchange_strong(Expected, EngineState_Stopping);
}

X_END
