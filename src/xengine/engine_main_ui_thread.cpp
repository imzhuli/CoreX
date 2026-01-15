#include "./engine_main_ui_thread.hpp"

#include "../wsi/wsi.hpp"
#include "./engine_render_thread.hpp"
#include "base.hpp"

X_BEGIN

void InitMainUIThread() {
	XEMainThreadChecker.Assert();
}

void CleanMainUIThread() {
	XEMainThreadChecker.Assert();
}

void MainUIThreadLoop() {
	while (XERunState) {
		WSILoopOnce();
		if (WSIHasDeferredCommands()) {
			auto H = AcquireNoRenderSession();
			WSIProcessedCommandQueue();
			ReleaseNoRenderSession(std::move(H));
		}
	}
}

X_END
