#pragma once
#include "./vk.hpp"

#include <string>
#include <vector>

X_BEGIN

X_PRIVATE std::string ToString(VkResult result);
X_PRIVATE std::string ToString(VkFormat format);
X_PRIVATE std::string ToString(VkColorSpaceKHR ColorSpace);
X_PRIVATE std::string ToString(VkPresentModeKHR present_mode);
X_PRIVATE std::string ToString(VkSurfaceTransformFlagBitsKHR transform_flag);
X_PRIVATE std::string ToString(VkSurfaceFormatKHR surface_format);
X_PRIVATE std::string ToString(VkCompositeAlphaFlagBitsKHR composite_alpha);
X_PRIVATE std::string ToString(VkImageUsageFlagBits image_usage);
X_PRIVATE std::string ToString(bool flag);

VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type,
	const VkDebugUtilsMessengerCallbackDataEXT * callback_data, void * user_data
);

VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback(
	VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT /*type*/, uint64_t /*object*/, size_t /*location*/, int32_t /*message_code*/,
	const char * layer_prefix, const char * message, void * /*user_data*/
);

std::vector<const char *> GetOptimalValidationLayers(const std::vector<VkLayerProperties> & supported_instance_layers);

X_END