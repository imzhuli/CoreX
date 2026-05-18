#include "./renderer.hpp"

#include "../core/indexed_storage_static.hpp"
#include "../core/thread.hpp"
#include "../vk/vk_debug.hpp"
#include "../vk/vk_init.hpp"
#include "../xengine/engine.hpp"

#include <cmath>

X_BEGIN

static auto RendererPool       = xIndexedStorageStatic<xRenderer, 256>();
static auto NewRendererList    = xRendererList();
static auto UpdateRendererList = xRendererList();

bool xRenderer::Init(VkSurfaceKHR && Surface) {
	NativeSurfaceHandle = Steal(Surface, VK_NULL_HANDLE);
	return true;
}

void xRenderer::Clean() {
	vkDestroySurfaceKHR(VulkanInstance, Steal(NativeSurfaceHandle, VK_NULL_HANDLE), nullptr);
	return;
}

bool xRenderer::CreateSwapchain() {
	return false;
}

void xRenderer::DestroySwapchain() {
}

bool xRenderer::CreateCommands() {
	return true;
}

void xRenderer::DestroyCommands() {
}

bool xRenderer::CreateSyncStructures() {
	return true;
}

void xRenderer::DestroySyncStructures() {
}

bool xRenderer::CreateDefaultRenderPass() {
	return true;
}

void xRenderer::DestroyDefaultRenderPass() {
}

bool xRenderer::CreateFrameBuffers() {
	return true;
}

void xRenderer::DestroyFrameBuffers() {
}

bool xRenderer::RecreateSwapchain() {
	return true;
}

void xRenderer::Render() {
	return;
}

uint64_t xRenderer::Create(VkSurfaceKHR && Surface) {
	auto RendererId = RendererPool.Acquire();
	if (!RendererId) {
		return {};
	}
	auto R = &RendererPool[RendererId];
	if (!R->Init(std::move(Surface))) {
		RendererPool.Release(RendererId);
		return {};
	}
	R->RendererId = RendererId;
	NewRendererList.AddTail(*R);
	return RendererId;
}

void xRenderer::UpdateAll() {
	// Process new renderers

	// update renderers:
}

void xRenderer::Destroy(uint64_t RendererId) {
	auto R = RendererPool.CheckAndGet(RendererId);
	assert(R && R->RendererId == RendererId);
	R->Clean();
	RendererPool.Release(RendererId);
}

X_END
