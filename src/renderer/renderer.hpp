#pragma once
#include "../core/core_min.hpp"
#include "../core/list.hpp"
#include "../vk/vk.hpp"

X_BEGIN

struct xRendererListNode : xListNode {};
using xRendererList = xList<xRendererListNode>;

class xRenderer
	: xRendererListNode
	, xAbstract {
private:
	X_API_MEMBER virtual bool Init(VkSurfaceKHR Surface);
	X_API_MEMBER virtual void Clean();
	X_API_MEMBER virtual void Render();

protected:
	X_PRIVATE_MEMBER bool CreateSwapchain();
	X_PRIVATE_MEMBER void DestroySwapchain();
	X_PRIVATE_MEMBER bool RecreateSwapchain();

	X_PRIVATE_MEMBER bool CreateCommands();
	X_PRIVATE_MEMBER void DestroyCommands();

	X_PRIVATE_MEMBER bool CreateSyncStructures();
	X_PRIVATE_MEMBER void DestroySyncStructures();

private:
	X_PRIVATE_MEMBER bool CreateDefaultRenderPass();
	X_PRIVATE_MEMBER void DestroyDefaultRenderPass();
	X_PRIVATE_MEMBER bool CreateFrameBuffers();
	X_PRIVATE_MEMBER void DestroyFrameBuffers();

public:
	X_PRIVATE_STATIC_MEMBER bool Spawn(VkSurfaceKHR && Surface);
	X_PRIVATE_STATIC_MEMBER void UpdateAll();
	X_PRIVATE_STATIC_MEMBER void CleanAll();

private:
	VkSurfaceKHR Surface = {};
	vkb::Device  Device  = {};

	bool                       SwapchainDirty       = true;
	vkb::Swapchain             Swapchain            = {};
	VkFormat                   SwapchainImageFormat = {};
	std::vector<VkImage>       SwapchainImages      = {};
	std::vector<VkImageView>   SwapchainImageViews  = {};
	VkExtent2D                 SwapchainExtent      = {};
	std::vector<VkFramebuffer> FrameBuffers         = {};

	VkQueue         GraphicsQueue       = {};
	uint32_t        GraphicsQueueFamily = {};
	VkCommandPool   CommandPool         = {};
	VkCommandBuffer MainCommandBuffer   = {};

	VkRenderPass RenderPass = {};

	VkSemaphore PresentSemaphore;
	VkSemaphore RenderSemaphore;
	VkFence     RenderFence;

	size_t FrameNumber = {};
};

X_END
