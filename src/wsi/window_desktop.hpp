#pragma once
#include "../vk/vk.hpp"
#include "./wsi_desktop.hpp"

#ifdef X_SYSTEM_DESKTOP
X_BEGIN

struct xNativeWindowHandle {
	uint64_t  WindowId     = {};
	xVariable NativeHandle = {};
};

X_PRIVATE void UpdateWindows(uint64_t TimestampMS);

X_END
#endif
