#include "./vk_init.hpp"

X_BEGIN

namespace vkinit {

	VkCommandPoolCreateInfo CommandPoolCreateInfo(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags /*= 0*/) {
		VkCommandPoolCreateInfo info = {};
		info.sType                   = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.pNext                   = nullptr;
		info.queueFamilyIndex        = queueFamilyIndex;
		info.flags                   = flags;
		return info;
	}

	VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool pool, uint32_t count /*= 1*/) {
		VkCommandBufferAllocateInfo info = {};
		info.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		info.pNext                       = nullptr;
		info.commandPool                 = pool;
		info.commandBufferCount          = count;
		info.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		return info;
	}

	VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags flags /*= 0*/) {
		VkFenceCreateInfo info = {};
		info.sType             = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		info.pNext             = nullptr;
		info.flags             = flags;
		return info;
	}

	VkSemaphoreCreateInfo SemaphoreCreateInfo(VkSemaphoreCreateFlags flags /*= 0*/) {
		VkSemaphoreCreateInfo info = {};
		info.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		info.pNext                 = nullptr;
		info.flags                 = flags;
		return info;
	}

	VkSemaphoreSignalInfo SemaphoreSignalInfo(VkSemaphore semaphore, uint64_t value) {
		VkSemaphoreSignalInfo info = {};
		info.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO;
		info.semaphore             = semaphore;
		info.value                 = 1;
		return info;
	}

	VkSemaphoreWaitInfo SemaphoreWaitInfo(VkSemaphore * psemaphore) {
		static uint64_t     value = 1;
		VkSemaphoreWaitInfo info  = {};
		info.sType                = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO;
		info.pSemaphores          = psemaphore;
		info.pValues              = &value;
		info.semaphoreCount       = 1;
		return info;
	}

	VkCommandBufferBeginInfo CommandBufferBeginInfo(VkCommandBufferUsageFlags flags /*= 0*/) {
		VkCommandBufferBeginInfo info = {};
		info.sType                    = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.pNext                    = nullptr;

		info.pInheritanceInfo = nullptr;
		info.flags            = flags;
		return info;
	}

	VkImageSubresourceRange ImageSubresourceRange(VkImageAspectFlags aspectMask) {
		VkImageSubresourceRange subImage{};
		subImage.aspectMask     = aspectMask;
		subImage.baseMipLevel   = 0;
		subImage.levelCount     = VK_REMAINING_MIP_LEVELS;
		subImage.baseArrayLayer = 0;
		subImage.layerCount     = VK_REMAINING_ARRAY_LAYERS;

		return subImage;
	}

}  // namespace vkinit

X_END
