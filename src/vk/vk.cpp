#include "./vk.hpp"

#include "../3rd/vk_bootstrap/VkBootstrap.h"
#include "../wsi/wsi_desktop.hpp"
#include "../xengine/engine.hpp"
#include "./vk_debug.hpp"
#include "vulkan/vulkan_core.h"

#include <sstream>

X_BEGIN

vkb::Instance VkbInstance    = {};
VkInstance    VulkanInstance = {};

namespace {
	std::vector<const char *> VulkanExpectedExtensions     = {};
	bool                      VulkanEnableValidationLayers = {};

}  // namespace

static void ResetVulkanOptions() {
	Reset(VulkanExpectedExtensions);
#ifndef NDEBUG
	VulkanEnableValidationLayers = true;
#endif
}

static bool InitGlfwVulkanExtensions() {
#ifdef X_SYSTEM_DESKTOP
	uint32_t      glfwExtensionCount = 0;
	const char ** glfwExtensions     = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	if (!glfwExtensions && glfwExtensionCount > 0) {
		XELogger->E("Failed to get glfw vulkan extensions");
		return false;
	}
	for (uint32_t I = 0; I < glfwExtensionCount; ++I) {
		VulkanExpectedExtensions.push_back(glfwExtensions[I]);
	}
	return true;
#else
	return false;
#endif
}

bool InitVulkan(const char * ApplicationName) {

	ResetVulkanOptions();
	if (!InitGlfwVulkanExtensions()) {
		return false;
	}

	vkb::InstanceBuilder Builder;
	Builder.set_app_name(ApplicationName);
	Builder.set_engine_name("XEngine");
	Builder.require_api_version(VK_API_VERSION_1_2);
	Builder.enable_validation_layers(VulkanEnableValidationLayers);
	Builder.set_headless(true);
	// instance_builder.use_default_debug_messenger();
	// instance_builder.request_validation_layers();
	// instance_builder.enable_extensions(VulkanExpectedExtensions);
	auto instance_ret = Builder.build();
	if (!instance_ret) {
		return false;
	}

	VkbInstance    = *instance_ret;
	VulkanInstance = instance_ret->instance;

	// uint32_t ext_count = 0;
	// vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, nullptr);
	// std::vector<VkExtensionProperties> extensions(ext_count);
	// vkEnumerateInstanceExtensionProperties(nullptr, &ext_count, extensions.data());

	// std::cout << "\n=== 已启用的 Instance Extensions ===" << std::endl;
	// for (const auto & ext : extensions) {
	// 	std::cout << " - " << ext.extensionName << " (版本: " << ext.specVersion << ")" << std::endl;
	// }

	// // 3. 查看已启用的 Instance Layers
	// uint32_t layer_count = 0;
	// vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
	// std::vector<VkLayerProperties> layers(layer_count);
	// vkEnumerateInstanceLayerProperties(&layer_count, layers.data());

	// std::cout << "\n=== 已启用的 Instance Layers ===" << std::endl;
	// for (const auto & layer : layers) {
	// 	std::cout << " - " << layer.layerName << " (版本: " << layer.specVersion << ")" << std::endl;
	// }

	return true;
}

void CleanVulkan() {
	Reset(VulkanInstance, VK_NULL_HANDLE);
	vkb::destroy_instance(Steal(VkbInstance));
	ResetVulkanOptions();
}

X_END
