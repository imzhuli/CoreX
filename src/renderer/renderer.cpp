#include "./renderer.hpp"

#include "../core/thread.hpp"
#include "../vk/vk_debug.hpp"
#include "../vk/vk_init.hpp"
#include "../xengine/engine.hpp"

#include <cmath>

X_BEGIN

static auto RendererListLock   = xSpinlock();
static auto NewRendererList    = xRendererList();
static auto UpdateRendererList = xRendererList();
static auto DeleteRendererList = xRendererList();

bool xRenderer::Init(VkSurfaceKHR Surface_) {

	return true;
}

void xRenderer::Clean() {
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
	return true;
}

void xRenderer::UpdateAll() {
}

void xRenderer::CleanAll() {
}

X_END
