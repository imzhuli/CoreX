#pragma once
#include "../core/core_min.hpp"
#include "../vk/vk.hpp"

X_BEGIN

struct xFrameData {
	VkCommandPool   CommandPool       = {};
	VkCommandBuffer MainCommandBuffer = {};

	VkSemaphore SwapchainSemaphore;
	VkSemaphore RenderSemaphore;
	VkFence     RenderFence;
};

constexpr size_t FRAME_OVERLAP = 2;

X_END
