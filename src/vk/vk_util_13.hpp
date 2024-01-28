#pragma once
#include "./vk.hpp"

X_BEGIN

namespace vkutil {

	// vulkan 1.3
	X_PRIVATE void TransitionImage(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout);

}  // namespace vkutil

X_END
