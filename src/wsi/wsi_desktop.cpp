#include "./wsi_desktop.hpp"

#include "./window_desktop.hpp"

#include <mutex>

#ifdef X_SYSTEM_DESKTOP

X_BEGIN

static bool       Inited = false;
static std::mutex InitMutex;

bool InitWSI() {
	auto InitGuard = std::lock_guard(InitMutex);
#ifdef GLFW_SUPPORT_JOYSTICK_HAT
	glfwInitHint(GLFW_JOYSTICK_HAT_BUTTONS, GLFW_FALSE);
#endif
	if (!glfwInit()) {
		return false;
	}
	X_DEBUG_PRINTF("GLFW vulkan support: %s", YN(glfwVulkanSupported()));
	Inited = true;
	return true;
}

void WSILoopOnce(uint_fast32_t TimeoutMS) {
	UpdateWindows(TimeoutMS);
}

void WSILoopClean() {
	CleanupDyingWindows();
}

void CleanWSI() {
	CleanupDyingWindows();
	auto InitGuard = std::lock_guard(InitMutex);
	glfwTerminate();
	Inited = false;
	return;
}

X_END
#endif
