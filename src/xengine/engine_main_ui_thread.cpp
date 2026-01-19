#include "./engine_main_ui_thread.hpp"

#include "../wsi/wsi.hpp"
#include "./engine.hpp"
#include "./engine_render_thread.hpp"

X_BEGIN

void InitMainUIThread() {
	XE_ASSERT_MAIN_THREAD();
	InitRenderThread();
}

void CleanMainUIThread() {
	XE_ASSERT_MAIN_THREAD();
	CleanRenderThread();
}

void MainUIThreadLoop() {
	auto RT = std::thread(RenderThreadLoop);
	while (XERunState) {
		WSILoopOnce();
		if (WSIHasDeferredCommands()) {
			auto H = AcquireNoRenderSession();
			WSIProcessedCommandQueue();
			ReleaseNoRenderSession(std::move(H));
		}
	}
	StopRenderThreadLoop();
	RT.join();
}

X_END
