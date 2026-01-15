#include "./renderer.hpp"

#include "../core/thread.hpp"
#include "../vk/vk_debug.hpp"
#include "../vk/vk_init.hpp"
#include "../xengine/engine.hpp"

#include <cmath>

X_BEGIN

static auto NewRendererList    = xRendererList();
static auto UpdateRendererList = xRendererList();
static auto DeleteRendererList = xRendererList();

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

bool xRenderer::Spawn(VkSurfaceKHR && Surface) {
	auto R = new xRenderer();
	if (!R->Init(std::move(Surface))) {
		delete R;
		return false;
	}
	NewRendererList.AddTail(*R);
	return true;
}

void xRenderer::UpdateAll() {
	// Process new renderers

	// update renderers:

	// destroy dying renderers
	DestroyDyingRenderers();
}

void xRenderer::CleanAll() {
	DeleteRendererList.GrabListTail(NewRendererList);
	DeleteRendererList.GrabListTail(UpdateRendererList);
	DestroyDyingRenderers();
}

void xRenderer::DeferDestroyRenderer(xRenderer * R) {
	DeleteRendererList.GrabTail(*R);
}

void xRenderer::DestroyDyingRenderers() {
	while (auto P = static_cast<xRenderer *>(DeleteRendererList.PopHead())) {
		P->Clean();
		delete P;
	}
}

X_END
