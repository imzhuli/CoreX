#pragma once
#include "./vk.hpp"

X_BEGIN

namespace vkinit {

	X_PRIVATE VkCommandPoolCreateInfo     CommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags = 0);
	X_PRIVATE VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count = 1);
	X_PRIVATE VkFenceCreateInfo           FenceCreateInfo(VkFenceCreateFlags flags = 0);
	X_PRIVATE VkSemaphoreCreateInfo       SemaphoreCreateInfo(VkSemaphoreCreateFlags flags = 0);
	X_PRIVATE VkSemaphoreWaitInfo         SemaphoreWaitInfo(VkSemaphore * psemaphore);
	X_PRIVATE VkSemaphoreSignalInfo       SemaphoreSignalInfo(VkSemaphore semaphore, uint64_t value = 1);
	X_PRIVATE VkCommandBufferBeginInfo    CommandBufferBeginInfo(VkCommandBufferUsageFlags flags = 0);
	X_PRIVATE VkImageSubresourceRange     ImageSubresourceRange(VkImageAspectFlags aspectMask);

}  // namespace vkinit

X_END
