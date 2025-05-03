#include "./renderer.hpp"

#include "../3rd/vk_bootstrap/VkBootstrap.h"
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

	auto SurfaceGuard = xValueGuard(Surface, Surface_);

	// vulkan 1.3 features
	VkPhysicalDeviceVulkan13Features features13{};
	features13.dynamicRendering = true;
	features13.synchronization2 = true;

	// vulkan 1.2 features
	VkPhysicalDeviceVulkan12Features features12{};
	features12.bufferDeviceAddress = true;
	features12.descriptorIndexing  = true;

	// use vkbootstrap to select a gpu.
	auto selector      = vkb::PhysicalDeviceSelector{ VulkanInstance };
	auto select_result = selector.set_minimum_version(X_MIN_VK_VERSION_MAJOR, X_MIN_VK_VERSION_MINOR)
							 .set_required_features_13(features13)
							 .set_required_features_12(features12)
							 .set_surface(Surface)
							 .select();
	if (!select_result.has_value()) {
		auto Error = (vkb::PhysicalDeviceError)select_result.error().value();
		X_DEBUG_PRINTF("error: %s", vkb::to_string(Error));
		Touch(Error);
		return false;
	}
	auto ChosenGPU     = select_result.value();
	auto DeviceBuilder = vkb::DeviceBuilder(ChosenGPU);

	auto DeviceResult = DeviceBuilder.build();
	if (!DeviceResult.has_value()) {
		return false;
	}

	Device              = DeviceResult.value();
	GraphicsQueue       = Device.get_queue(vkb::QueueType::graphics).value();
	GraphicsQueueFamily = Device.get_queue_index(vkb::QueueType::graphics).value();
	auto DeviceGuard    = xScopeGuard([this] {
        Reset(GraphicsQueue);
        Reset(GraphicsQueueFamily);
        destroy_device(Steal(Device));
    });

	X_DEBUG_PRINTF("GraphicQueue: %p, GraphicsQueueFamily: %u", (void *)GraphicsQueue, (unsigned)GraphicsQueueFamily);

	if (!CreateCommands()) {
		return false;
	}
	auto CommandGuard = xScopeGuard([this] { DestroyCommands(); });

	if (!CreateSyncStructures()) {
		return false;
	}
	auto SyncGuard = xScopeGuard([this] { DestroySyncStructures(); });

	CreateSwapchain();

	Reset(FrameNumber);
	SurfaceGuard.Dismiss();
	DeviceGuard.Dismiss();
	CommandGuard.Dismiss();
	SyncGuard.Dismiss();

	vkDeviceWaitIdle(Device);  // an extra wait, so the renderer object become safe to be passed between threads
	X_DEBUG_PRINTF("finished device created: %p", (void *)Device.device);
	return true;
}

void xRenderer::Clean() {
	[[maybe_unused]] auto DeviceShadow = Device.device;
	Reset(FrameNumber);
	DestroySwapchain();
	DestroySyncStructures();
	DestroyCommands();
	Reset(GraphicsQueue);
	Reset(GraphicsQueueFamily);
	destroy_device(Steal(Device));
	vkDestroySurfaceKHR(VulkanInstance, Steal(Surface), nullptr);
	X_DEBUG_PRINTF("finished device: %p", (void *)DeviceShadow);
	return;
}

bool xRenderer::CreateSwapchain() {
	assert(!Swapchain.swapchain);
	SwapchainDirty = true;

	auto SurfaceCaps = VkSurfaceCapabilitiesKHR{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device.physical_device, Surface, &SurfaceCaps);
	auto width  = SurfaceCaps.currentExtent.width;
	auto height = SurfaceCaps.currentExtent.height;

	auto FormatGuard = xValueGuard(SwapchainImageFormat, VK_FORMAT_B8G8R8A8_UNORM);

	auto Builder     = vkb::SwapchainBuilder{ Device, Surface };
	auto BuildResult = Builder
						   .set_desired_format(VkSurfaceFormatKHR{
							   .format     = SwapchainImageFormat,
							   .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
						   })
						   // use vsync present mode
						   .set_desired_present_mode(VK_PRESENT_MODE_MAILBOX_KHR)
						   .set_desired_extent(width, height)
						   .add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
						   .build();

	if (!BuildResult.has_value()) {
		X_DEBUG_PRINTF("failed to build swapchain");
		return false;
	}
	Swapchain           = BuildResult.value();
	auto SwapchainGuard = xScopeGuard([this] { vkb::destroy_swapchain(Steal(Swapchain)); });

	SwapchainExtent = Swapchain.extent;
	// store swapchain and its related images
	SwapchainImages               = Swapchain.get_images().value();
	auto SwapchainImageViewResult = Swapchain.get_image_views();
	if (!SwapchainImageViewResult.has_value()) {
		X_DEBUG_PRINTF("failed to get swapchain images");
		return false;
	}
	SwapchainImageViews = SwapchainImageViewResult.value();

	if (!CreateDefaultRenderPass()) {
		X_DEBUG_PRINTF("failed to create renderppass");
		return false;
	}
	auto RenderPassGuard = xScopeGuard([this] { DestroyDefaultRenderPass(); });

	if (!CreateFrameBuffers()) {
		X_DEBUG_PRINTF("failed to create framebuffers");
		return false;
	}
	auto FrameBufferGuard = xScopeGuard([this] { DestroyFrameBuffers(); });

	FormatGuard.Dismiss();
	SwapchainGuard.Dismiss();
	FrameBufferGuard.Dismiss();
	RenderPassGuard.Dismiss();

	SwapchainDirty = false;
	// X_DEBUG_PRINTF("finished");
	return true;
}

void xRenderer::DestroySwapchain() {
	if (!Swapchain) {
		assert(SwapchainDirty);
		return;
	}
	vkDeviceWaitIdle(Device);
	DestroyFrameBuffers();
	DestroyDefaultRenderPass();
	Swapchain.destroy_image_views(SwapchainImageViews);
	Renew(SwapchainImageViews);
	Renew(SwapchainImages);
	vkb::destroy_swapchain(Steal(Swapchain));
	Reset(SwapchainImageFormat);

	SwapchainDirty = true;
	// X_DEBUG_PRINTF("finished");
}

bool xRenderer::CreateCommands() {
	auto CommandPoolInfo = vkinit::CommandPoolCreateInfo(GraphicsQueueFamily, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	auto PoolResult      = vkCreateCommandPool(Device, &CommandPoolInfo, nullptr, &CommandPool);
	if (PoolResult != VK_SUCCESS) {
		return false;
	}
	auto PoolGuard = xScopeGuard([this] { vkDestroyCommandPool(Device, Steal(CommandPool), nullptr); });

	// allocate the default command buffer that we will use for rendering
	auto CmdAllocInfo    = vkinit::CommandBufferAllocateInfo(CommandPool);
	auto CmdBufferResult = vkAllocateCommandBuffers(Device, &CmdAllocInfo, &MainCommandBuffer);
	if (CmdBufferResult != VK_SUCCESS) {
		return false;
	}
	PoolGuard.Dismiss();
	return true;
}

void xRenderer::DestroyCommands() {
	vkDeviceWaitIdle(Device);
	Reset(MainCommandBuffer);  // command buffer will be destroyed by deleting commandpool
	vkDestroyCommandPool(Device, Steal(CommandPool), nullptr);
}

bool xRenderer::CreateSyncStructures() {
	auto FenceCreateInfo     = vkinit::FenceCreateInfo(VK_FENCE_CREATE_SIGNALED_BIT);
	auto SemaphoreCreateInfo = vkinit::SemaphoreCreateInfo();

	auto FenceResult = vkCreateFence(Device, &FenceCreateInfo, nullptr, &RenderFence);
	if (FenceResult) {
		X_DEBUG_PRINTF("vkCreateFence error: %s", ToString(FenceResult).c_str());
		return false;
	}
	auto FenceGuard = xScopeGuard([this] { vkDestroyFence(Device, Steal(RenderFence), nullptr); });

	auto PresentSemaphoreResult = vkCreateSemaphore(Device, &SemaphoreCreateInfo, nullptr, &PresentSemaphore);
	if (PresentSemaphoreResult) {
		X_DEBUG_PRINTF("vkCreateSemaphore error: %s", ToString(PresentSemaphoreResult).c_str());
		return false;
	}
	auto PresentSemaphoreGuard = xScopeGuard([this] { vkDestroySemaphore(Device, Steal(PresentSemaphore), nullptr); });

	auto RenderSemaphoreResult = (vkCreateSemaphore(Device, &SemaphoreCreateInfo, nullptr, &RenderSemaphore));
	if (RenderSemaphoreResult) {
		X_DEBUG_PRINTF("vkCreateSemaphore error: %s", ToString(RenderSemaphoreResult).c_str());
		return false;
	}
	auto RenderSemaphoreGuard = xScopeGuard([this] { vkDestroySemaphore(Device, Steal(RenderSemaphore), nullptr); });

	FenceGuard.Dismiss();
	PresentSemaphoreGuard.Dismiss();
	RenderSemaphoreGuard.Dismiss();
	return true;
}

void xRenderer::DestroySyncStructures() {
	vkDeviceWaitIdle(Device);
	vkDestroySemaphore(Device, Steal(RenderSemaphore), nullptr);
	vkDestroySemaphore(Device, Steal(PresentSemaphore), nullptr);
	vkDestroyFence(Device, Steal(RenderFence), nullptr);
}

bool xRenderer::CreateDefaultRenderPass() {
	// the renderpass will use this color attachment.
	VkAttachmentDescription color_attachment = {};
	// the attachment will have the format needed by the swapchain
	color_attachment.format = SwapchainImageFormat;
	// 1 sample, we won't be doing MSAA
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	// we Clear when this attachment is loaded
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	// we keep the attachment stored when the renderpass ends
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	// we don't care about stencil
	color_attachment.stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	// we don't know or care about the starting layout of the attachment
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	// after the renderpass ends, the image has to be on a layout ready for display
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref = {};
	// attachment number will index into the pAttachments array in the parent renderpass itself
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// we are going to create 1 subpass, which is the minimum you can do
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments    = &color_attachment_ref;

	VkRenderPassCreateInfo render_pass_info = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
	};

	// connect the color attachment to the info
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments    = &color_attachment;
	// connect the subpass to the info
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses   = &subpass;

	if (VK_SUCCESS != vkCreateRenderPass(Device, &render_pass_info, nullptr, &RenderPass)) {
		return false;
	}
	return true;
}

void xRenderer::DestroyDefaultRenderPass() {
	vkDestroyRenderPass(Device, Steal(RenderPass), nullptr);
}

bool xRenderer::CreateFrameBuffers() {
	// create the framebuffers for the swapchain images. This will connect the render-pass to the images for rendering
	VkFramebufferCreateInfo fb_info = {};
	fb_info.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fb_info.pNext                   = nullptr;

	fb_info.renderPass      = RenderPass;
	fb_info.attachmentCount = 1;
	fb_info.width           = SwapchainExtent.width;
	fb_info.height          = SwapchainExtent.height;
	fb_info.layers          = 1;

	// grab how many images we have in the swapchain
	assert(FrameBuffers.empty());
	const auto swapchain_imagecount = (uint32_t)SwapchainImages.size();

	// create framebuffers for each of the swapchain image views
	for (uint32_t i = 0; i < swapchain_imagecount; i++) {
		fb_info.pAttachments = &SwapchainImageViews[i];
		VkFramebuffer FrameBuffer;
		if (VK_SUCCESS != vkCreateFramebuffer(Device, &fb_info, nullptr, &FrameBuffer)) {
			break;
		}
		FrameBuffers.push_back(FrameBuffer);
	}
	if (FrameBuffers.size() != swapchain_imagecount) {
		DestroyFrameBuffers();
		return false;
	}
	return true;
}

void xRenderer::DestroyFrameBuffers() {
	for (auto FrameBuffer : FrameBuffers) {
		vkDestroyFramebuffer(Device, FrameBuffer, nullptr);
	}
	Renew(FrameBuffers);
}

bool xRenderer::RecreateSwapchain() {
	vkDeviceWaitIdle(Device);
	DestroySwapchain();
	return CreateSwapchain();
}

void xRenderer::Render() {

	VkResult Result;

	// wait until the gpu has finished rendering the last frame. Timeout of 1
	Result = vkWaitForFences(Device, 1, &RenderFence, true, UINT64_MAX);
	if (Result) {
		return;
	}
	// request image from the swapchain
	if (Steal(SwapchainDirty) && RecreateSwapchain()) {
		return;
	}

	uint32_t SwapchainImageIndex;
	Result = vkAcquireNextImageKHR(Device, Swapchain, UINT64_MAX, PresentSemaphore, nullptr, &SwapchainImageIndex);
	if (Result) {
		SwapchainDirty = true;
		if (Result != VK_SUBOPTIMAL_KHR) {
			X_DEBUG_PRINTF("vkAcquireNextImageKHR error: %s", ToString(Result).c_str());
			return;
		}
	}

	// reset & restart command buffer
	Result = vkResetCommandBuffer(MainCommandBuffer, 0);
	if (Result) {
		X_DEBUG_PRINTF("vkResetCommandBuffer error, %s", ToString(Result).c_str());
		return;
	}

	auto CmdBeginInfo = vkinit::CommandBufferBeginInfo(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	VkRuntimeAssert(vkBeginCommandBuffer(MainCommandBuffer, &CmdBeginInfo));
	{
		VkClearValue clearValue;
		float        flash = abs(sin(FrameNumber / 120.f));
		clearValue.color   = { { 0.0f, 0.0f, flash, 1.0f } };

		// start the main renderpass.
		// We will use the clear color from above, and the framebuffer of the index the swapchain gave us
		VkRenderPassBeginInfo rpInfo = {};
		rpInfo.sType                 = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		rpInfo.pNext                 = nullptr;

		rpInfo.renderPass          = RenderPass;
		rpInfo.renderArea.offset.x = 0;
		rpInfo.renderArea.offset.y = 0;
		rpInfo.renderArea.extent   = SwapchainExtent;
		rpInfo.framebuffer         = FrameBuffers[SwapchainImageIndex];

		// connect clear values
		rpInfo.clearValueCount = 1;
		rpInfo.pClearValues    = &clearValue;

		vkCmdBeginRenderPass(MainCommandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdEndRenderPass(MainCommandBuffer);
	}
	VkRuntimeAssert(vkEndCommandBuffer(MainCommandBuffer));

	// prepare the submission to the queue.
	// we want to wait on the _presentSemaphore, as that semaphore is signaled when the swapchain is ready
	// we will signal the _renderSemaphore, to signal that rendering has finished
	VkSubmitInfo submit = {};
	submit.sType        = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit.pNext        = nullptr;

	VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	submit.pWaitDstStageMask = &waitStage;

	submit.waitSemaphoreCount = 1;
	submit.pWaitSemaphores    = &PresentSemaphore;

	submit.signalSemaphoreCount = 1;
	submit.pSignalSemaphores    = &RenderSemaphore;

	submit.commandBufferCount = 1;
	submit.pCommandBuffers    = &MainCommandBuffer;

	// submit command buffer to the queue and execute it.
	//  _renderFence will now block until the graphic commands finish execution
	VkRuntimeAssert(vkResetFences(Device, 1, &RenderFence));
	Result = vkQueueSubmit(GraphicsQueue, 1, &submit, RenderFence);
	if (Result) {
		X_DEBUG_PRINTF("vkQueueSubmit error: %s", ToString(Result).c_str());
		return;
	}

	// this will put the image we just rendered into the visible window.
	// we want to wait on the _renderSemaphore for that,
	// as it's necessary that drawing commands have finished before the image is displayed to the user
	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType            = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext            = nullptr;

	presentInfo.pSwapchains    = &Swapchain.swapchain;
	presentInfo.swapchainCount = 1;

	presentInfo.pWaitSemaphores    = &RenderSemaphore;
	presentInfo.waitSemaphoreCount = 1;

	presentInfo.pImageIndices = &SwapchainImageIndex;

	Result = vkQueuePresentKHR(GraphicsQueue, &presentInfo);
	if (Result) {
		SwapchainDirty = true;
		if (Result != VK_SUBOPTIMAL_KHR) {
			X_DEBUG_PRINTF("vkQueuePresentKHR error: %s", ToString(Result).c_str());
			return;
		}
	}

	// increase the number of frames drawn
	++FrameNumber;
	return;
}

bool xRenderer::Spawn(VkSurfaceKHR && Surface) {
	auto Renderer = new xRenderer();
	if (!Renderer->Init(Surface)) {
		delete Renderer;
		return false;
	}
	do {
		auto ListGuard = xSpinlockGuard(RendererListLock);
		NewRendererList.AddTail(*Renderer);
	} while (false);
	Reset(Surface);
	return true;
}

void xRenderer::UpdateAll() {
	do {
		auto ListGuard = xSpinlockGuard(RendererListLock);
		UpdateRendererList.GrabListTail(NewRendererList);
	} while (false);

	UpdateRendererList.ForEach([](xRendererListNode & N) {
		auto & Renderer = static_cast<xRenderer &>(N);
		Renderer.Render();
	});

	while (auto RP = static_cast<xRenderer *>(DeleteRendererList.PopHead())) {
		RP->Clean();
		delete RP;
	}
}

void xRenderer::CleanAll() {
	do {
		auto ListGuard = xSpinlockGuard(RendererListLock);
		DeleteRendererList.GrabListTail(NewRendererList);
		DeleteRendererList.GrabListTail(UpdateRendererList);
	} while (false);

	while (auto RP = static_cast<xRenderer *>(DeleteRendererList.PopHead())) {
		RP->Clean();
		delete RP;
	}
}

X_END
