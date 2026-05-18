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
	X_API_MEMBER virtual bool Init(VkSurfaceKHR && Surface);
	X_API_MEMBER virtual void Clean();
	X_API_MEMBER virtual void Render();

protected:
	X_API_MEMBER bool CreateSwapchain();
	X_API_MEMBER void DestroySwapchain();
	X_API_MEMBER bool RecreateSwapchain();

	X_API_MEMBER bool CreateCommands();
	X_API_MEMBER void DestroyCommands();

	X_API_MEMBER bool CreateSyncStructures();
	X_API_MEMBER void DestroySyncStructures();

private:
	X_PRIVATE_MEMBER bool CreateDefaultRenderPass();
	X_PRIVATE_MEMBER void DestroyDefaultRenderPass();
	X_PRIVATE_MEMBER bool CreateFrameBuffers();
	X_PRIVATE_MEMBER void DestroyFrameBuffers();

public:
	X_PRIVATE_STATIC_MEMBER uint64_t Create(VkSurfaceKHR && Surface);
	X_PRIVATE_STATIC_MEMBER void     Destroy(uint64_t RendererId);
	X_PRIVATE_STATIC_MEMBER void     UpdateAll();

private:
	uint64_t     RendererId          = {};
	VkSurfaceKHR NativeSurfaceHandle = {};
};

X_END
