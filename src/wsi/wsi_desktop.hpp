#pragma once

#include "./wsi.hpp"

#ifdef X_SYSTEM_DESKTOP

#define GLFW_INCLUDE_VULKAN
#define GLFW_VULKAN_STATIC
#include <GLFW/glfw3.h>

#if GLFW_VERSION_MAJOR > 3
#define GLFW_SUPPORT_JOYSTICK_HAT true
#elif GLFW_VERSION_MAJOR == 3
#if GLFW_VERSION_MINOR >= 3
#define GLFW_SUPPORT_JOYSTICK_HAT true
#endif
#endif

#endif