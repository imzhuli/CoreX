#pragma once
#include "../core/core_min.hpp"

#include <vulkan/vulkan.h>

X_BEGIN

#define X_MIN_VK_VERSION_MAJOR 1
#define X_MIN_VK_VERSION_MINOR 2

X_PRIVATE VkInstance VulkanInstance;

X_PRIVATE bool InitVulkan(const char * ApplicationName = "XEApplication");
X_PRIVATE void CleanVulkan();
X_INLINE void  VkRuntimeAssert(VkResult Result) {
    X_RUNTIME_ASSERT(VK_SUCCESS == Result);
}

X_END
