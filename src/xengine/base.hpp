#pragma once
#include "../core/core_min.hpp"
#include "../core/logger.hpp"

#include <optional>

X_BEGIN

using xHandle = std::optional<void *>;

X_PRIVATE xLogger *           XELogger;  // First object inited only available after init is done.
X_PRIVATE xRunState           XERunState;
X_PRIVATE xThreadSynchronizer XECleanupSynchronizer;

X_END
