#pragma once
#include "../core/core_min.hpp"
#include "../core/logger.hpp"

X_BEGIN

struct xHandle {
	xVariable Native;
};
X_PRIVATE xLogger *           XELogger;  // First object inited only available after init is done.
X_PRIVATE xRunState           XERunState;
X_PRIVATE xThreadChecker      XEMainThreadChecker;
X_PRIVATE xThreadSynchronizer XECleanupSynchronizer;

X_END
