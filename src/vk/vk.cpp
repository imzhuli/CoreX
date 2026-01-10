#include "./vk.hpp"

#include "../wsi/wsi_desktop.hpp"
#include "../xengine/engine.hpp"
#include "./vk_debug.hpp"

#include <sstream>

X_BEGIN

VkInstance VulkanInstance = {};

namespace {
	VkApplicationInfo               VulkanAppInfo                  = {};
	std::vector<const char *>       VulkanExpectedExtensions       = {};
	bool                            VulkanEnableValidationLayers   = {};
	const std::vector<const char *> VulkanExpectedValidationLayers = {
		"VK_LAYER_KHRONOS_validation",
	};
	VkInstanceCreateInfo VulkanCreateInfo = {};

}  // namespace

static void ResetVulkanOptions() {
	Reset(VulkanAppInfo);
	Reset(VulkanExpectedExtensions);
	Reset(VulkanCreateInfo);
#ifndef NDEBUG
	VulkanEnableValidationLayers = true;
#endif
}

static void InitVulkanAppInfo(const char * ApplicationName) {
	VulkanAppInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;  // Vulkan结构体必须指定类型
	VulkanAppInfo.pApplicationName   = ApplicationName;
	VulkanAppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);  // 版本号封装
	VulkanAppInfo.pEngineName        = "XEngine";
	VulkanAppInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
	VulkanAppInfo.apiVersion         = VK_API_VERSION_1_2;  // 明确使用Vulkan 1.2
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

static void DebugOutputExpectedExtensions() {
#ifndef NDEBUG
	auto OS = std::stringstream();
	for (auto & S : VulkanExpectedExtensions) {
		OS << "\t" << S << std::endl;
	}
	XELogger->D("ExpectedVulkanExtensions: \n%s", OS.str().c_str());
#endif
}

static bool InitValidationLayerSupport() {
	if (!VulkanEnableValidationLayers) {
		return true;
	}

	uint32_t LayerCount = 0;
	vkEnumerateInstanceLayerProperties(&LayerCount, nullptr);

	std::vector<VkLayerProperties> AvailableLayers(LayerCount);
	vkEnumerateInstanceLayerProperties(&LayerCount, AvailableLayers.data());

	for (auto & LayerName : VulkanExpectedValidationLayers) {
		bool LayerFound = false;
		for (const auto & LayerProps : AvailableLayers) {
			if (strcmp(LayerName, LayerProps.layerName) == 0) {
				LayerFound = true;
				break;
			}
		}
		if (!LayerFound) {
			XELogger->E("Validation layer not found: %s", LayerName);
			return false;
		}
	}
	return true;
}

static void DebugOutputExpectedValidationLayers() {
#ifndef NDEBUG
	auto OS = std::stringstream();
	for (auto & S : VulkanExpectedValidationLayers) {
		OS << "\t" << S << std::endl;
	}
	XELogger->D("VulkanExpectedValidationLayers: \n%s", OS.str().c_str());
#endif
}

static void InitVulkanCreateInfo() {
	VulkanCreateInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	VulkanCreateInfo.pApplicationInfo        = &VulkanAppInfo;
	VulkanCreateInfo.enabledExtensionCount   = (uint32_t)VulkanExpectedExtensions.size();
	VulkanCreateInfo.ppEnabledExtensionNames = VulkanExpectedExtensions.data();
	if (VulkanEnableValidationLayers) {
		VulkanCreateInfo.enabledLayerCount   = static_cast<uint32_t>(VulkanExpectedValidationLayers.size());
		VulkanCreateInfo.ppEnabledLayerNames = VulkanExpectedValidationLayers.data();
	}
}

bool InitVulkan(const char * ApplicationName) {
	ResetVulkanOptions();
	InitVulkanAppInfo(ApplicationName);
	if (!InitGlfwVulkanExtensions()) {
		return false;
	}
	DebugOutputExpectedExtensions();
	if (!InitValidationLayerSupport()) {
		return false;
	}
	DebugOutputExpectedValidationLayers();
	InitVulkanCreateInfo();

	VkResult Result = vkCreateInstance(&VulkanCreateInfo, nullptr, &VulkanInstance);
	if (Result != VK_SUCCESS) {
		XELogger->E("Failed to create VkInstance: error=%s", ToString(Result).c_str());
		return false;
	}

	return true;
}

void CleanVulkan() {
	vkDestroyInstance(Steal(VulkanInstance), nullptr);
	ResetVulkanOptions();
}

X_END
