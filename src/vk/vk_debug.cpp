#include "./vk_debug.hpp"

#include <cinttypes>

X_BEGIN

/**
 * ToString Serious
 */
#ifdef STR
#undef STR
#endif

std::string ToString(VkResult result) {
	switch (result) {
#define STR(r) \
	case r:    \
		return #r
		STR(VK_SUCCESS);
		STR(VK_NOT_READY);
		STR(VK_TIMEOUT);
		STR(VK_EVENT_SET);
		STR(VK_EVENT_RESET);
		STR(VK_INCOMPLETE);
		STR(VK_ERROR_OUT_OF_HOST_MEMORY);
		STR(VK_ERROR_OUT_OF_DEVICE_MEMORY);
		STR(VK_ERROR_INITIALIZATION_FAILED);
		STR(VK_ERROR_DEVICE_LOST);
		STR(VK_ERROR_MEMORY_MAP_FAILED);
		STR(VK_ERROR_LAYER_NOT_PRESENT);
		STR(VK_ERROR_EXTENSION_NOT_PRESENT);
		STR(VK_ERROR_FEATURE_NOT_PRESENT);
		STR(VK_ERROR_INCOMPATIBLE_DRIVER);
		STR(VK_ERROR_TOO_MANY_OBJECTS);
		STR(VK_ERROR_FORMAT_NOT_SUPPORTED);
		STR(VK_ERROR_SURFACE_LOST_KHR);
		STR(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);
		STR(VK_SUBOPTIMAL_KHR);
		STR(VK_ERROR_OUT_OF_DATE_KHR);
		STR(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR);
		STR(VK_ERROR_VALIDATION_FAILED_EXT);
		STR(VK_ERROR_INVALID_SHADER_NV);
#undef STR
		default:
			return "UNKNOWN_ERROR";
	}
}

std::string ToString(VkFormat format) {
	switch (format) {
		case VK_FORMAT_R4G4_UNORM_PACK8:
			return "VK_FORMAT_R4G4_UNORM_PACK8";
		case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
			return "VK_FORMAT_R4G4B4A4_UNORM_PACK16";
		case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
			return "VK_FORMAT_B4G4R4A4_UNORM_PACK16";
		case VK_FORMAT_R5G6B5_UNORM_PACK16:
			return "VK_FORMAT_R5G6B5_UNORM_PACK16";
		case VK_FORMAT_B5G6R5_UNORM_PACK16:
			return "VK_FORMAT_B5G6R5_UNORM_PACK16";
		case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
			return "VK_FORMAT_R5G5B5A1_UNORM_PACK16";
		case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
			return "VK_FORMAT_B5G5R5A1_UNORM_PACK16";
		case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
			return "VK_FORMAT_A1R5G5B5_UNORM_PACK16";
		case VK_FORMAT_R8_UNORM:
			return "VK_FORMAT_R8_UNORM";
		case VK_FORMAT_R8_SNORM:
			return "VK_FORMAT_R8_SNORM";
		case VK_FORMAT_R8_USCALED:
			return "VK_FORMAT_R8_USCALED";
		case VK_FORMAT_R8_SSCALED:
			return "VK_FORMAT_R8_SSCALED";
		case VK_FORMAT_R8_UINT:
			return "VK_FORMAT_R8_UINT";
		case VK_FORMAT_R8_SINT:
			return "VK_FORMAT_R8_SINT";
		case VK_FORMAT_R8_SRGB:
			return "VK_FORMAT_R8_SRGB";
		case VK_FORMAT_R8G8_UNORM:
			return "VK_FORMAT_R8G8_UNORM";
		case VK_FORMAT_R8G8_SNORM:
			return "VK_FORMAT_R8G8_SNORM";
		case VK_FORMAT_R8G8_USCALED:
			return "VK_FORMAT_R8G8_USCALED";
		case VK_FORMAT_R8G8_SSCALED:
			return "VK_FORMAT_R8G8_SSCALED";
		case VK_FORMAT_R8G8_UINT:
			return "VK_FORMAT_R8G8_UINT";
		case VK_FORMAT_R8G8_SINT:
			return "VK_FORMAT_R8G8_SINT";
		case VK_FORMAT_R8G8_SRGB:
			return "VK_FORMAT_R8G8_SRGB";
		case VK_FORMAT_R8G8B8_UNORM:
			return "VK_FORMAT_R8G8B8_UNORM";
		case VK_FORMAT_R8G8B8_SNORM:
			return "VK_FORMAT_R8G8B8_SNORM";
		case VK_FORMAT_R8G8B8_USCALED:
			return "VK_FORMAT_R8G8B8_USCALED";
		case VK_FORMAT_R8G8B8_SSCALED:
			return "VK_FORMAT_R8G8B8_SSCALED";
		case VK_FORMAT_R8G8B8_UINT:
			return "VK_FORMAT_R8G8B8_UINT";
		case VK_FORMAT_R8G8B8_SINT:
			return "VK_FORMAT_R8G8B8_SINT";
		case VK_FORMAT_R8G8B8_SRGB:
			return "VK_FORMAT_R8G8B8_SRGB";
		case VK_FORMAT_B8G8R8_UNORM:
			return "VK_FORMAT_B8G8R8_UNORM";
		case VK_FORMAT_B8G8R8_SNORM:
			return "VK_FORMAT_B8G8R8_SNORM";
		case VK_FORMAT_B8G8R8_USCALED:
			return "VK_FORMAT_B8G8R8_USCALED";
		case VK_FORMAT_B8G8R8_SSCALED:
			return "VK_FORMAT_B8G8R8_SSCALED";
		case VK_FORMAT_B8G8R8_UINT:
			return "VK_FORMAT_B8G8R8_UINT";
		case VK_FORMAT_B8G8R8_SINT:
			return "VK_FORMAT_B8G8R8_SINT";
		case VK_FORMAT_B8G8R8_SRGB:
			return "VK_FORMAT_B8G8R8_SRGB";
		case VK_FORMAT_R8G8B8A8_UNORM:
			return "VK_FORMAT_R8G8B8A8_UNORM";
		case VK_FORMAT_R8G8B8A8_SNORM:
			return "VK_FORMAT_R8G8B8A8_SNORM";
		case VK_FORMAT_R8G8B8A8_USCALED:
			return "VK_FORMAT_R8G8B8A8_USCALED";
		case VK_FORMAT_R8G8B8A8_SSCALED:
			return "VK_FORMAT_R8G8B8A8_SSCALED";
		case VK_FORMAT_R8G8B8A8_UINT:
			return "VK_FORMAT_R8G8B8A8_UINT";
		case VK_FORMAT_R8G8B8A8_SINT:
			return "VK_FORMAT_R8G8B8A8_SINT";
		case VK_FORMAT_R8G8B8A8_SRGB:
			return "VK_FORMAT_R8G8B8A8_SRGB";
		case VK_FORMAT_B8G8R8A8_UNORM:
			return "VK_FORMAT_B8G8R8A8_UNORM";
		case VK_FORMAT_B8G8R8A8_SNORM:
			return "VK_FORMAT_B8G8R8A8_SNORM";
		case VK_FORMAT_B8G8R8A8_USCALED:
			return "VK_FORMAT_B8G8R8A8_USCALED";
		case VK_FORMAT_B8G8R8A8_SSCALED:
			return "VK_FORMAT_B8G8R8A8_SSCALED";
		case VK_FORMAT_B8G8R8A8_UINT:
			return "VK_FORMAT_B8G8R8A8_UINT";
		case VK_FORMAT_B8G8R8A8_SINT:
			return "VK_FORMAT_B8G8R8A8_SINT";
		case VK_FORMAT_B8G8R8A8_SRGB:
			return "VK_FORMAT_B8G8R8A8_SRGB";
		case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
			return "VK_FORMAT_A8B8G8R8_UNORM_PACK32";
		case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
			return "VK_FORMAT_A8B8G8R8_SNORM_PACK32";
		case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
			return "VK_FORMAT_A8B8G8R8_USCALED_PACK32";
		case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
			return "VK_FORMAT_A8B8G8R8_SSCALED_PACK32";
		case VK_FORMAT_A8B8G8R8_UINT_PACK32:
			return "VK_FORMAT_A8B8G8R8_UINT_PACK32";
		case VK_FORMAT_A8B8G8R8_SINT_PACK32:
			return "VK_FORMAT_A8B8G8R8_SINT_PACK32";
		case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
			return "VK_FORMAT_A8B8G8R8_SRGB_PACK32";
		case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
			return "VK_FORMAT_A2R10G10B10_UNORM_PACK32";
		case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
			return "VK_FORMAT_A2R10G10B10_SNORM_PACK32";
		case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
			return "VK_FORMAT_A2R10G10B10_USCALED_PACK32";
		case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
			return "VK_FORMAT_A2R10G10B10_SSCALED_PACK32";
		case VK_FORMAT_A2R10G10B10_UINT_PACK32:
			return "VK_FORMAT_A2R10G10B10_UINT_PACK32";
		case VK_FORMAT_A2R10G10B10_SINT_PACK32:
			return "VK_FORMAT_A2R10G10B10_SINT_PACK32";
		case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
			return "VK_FORMAT_A2B10G10R10_UNORM_PACK32";
		case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
			return "VK_FORMAT_A2B10G10R10_SNORM_PACK32";
		case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
			return "VK_FORMAT_A2B10G10R10_USCALED_PACK32";
		case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
			return "VK_FORMAT_A2B10G10R10_SSCALED_PACK32";
		case VK_FORMAT_A2B10G10R10_UINT_PACK32:
			return "VK_FORMAT_A2B10G10R10_UINT_PACK32";
		case VK_FORMAT_A2B10G10R10_SINT_PACK32:
			return "VK_FORMAT_A2B10G10R10_SINT_PACK32";
		case VK_FORMAT_R16_UNORM:
			return "VK_FORMAT_R16_UNORM";
		case VK_FORMAT_R16_SNORM:
			return "VK_FORMAT_R16_SNORM";
		case VK_FORMAT_R16_USCALED:
			return "VK_FORMAT_R16_USCALED";
		case VK_FORMAT_R16_SSCALED:
			return "VK_FORMAT_R16_SSCALED";
		case VK_FORMAT_R16_UINT:
			return "VK_FORMAT_R16_UINT";
		case VK_FORMAT_R16_SINT:
			return "VK_FORMAT_R16_SINT";
		case VK_FORMAT_R16_SFLOAT:
			return "VK_FORMAT_R16_SFLOAT";
		case VK_FORMAT_R16G16_UNORM:
			return "VK_FORMAT_R16G16_UNORM";
		case VK_FORMAT_R16G16_SNORM:
			return "VK_FORMAT_R16G16_SNORM";
		case VK_FORMAT_R16G16_USCALED:
			return "VK_FORMAT_R16G16_USCALED";
		case VK_FORMAT_R16G16_SSCALED:
			return "VK_FORMAT_R16G16_SSCALED";
		case VK_FORMAT_R16G16_UINT:
			return "VK_FORMAT_R16G16_UINT";
		case VK_FORMAT_R16G16_SINT:
			return "VK_FORMAT_R16G16_SINT";
		case VK_FORMAT_R16G16_SFLOAT:
			return "VK_FORMAT_R16G16_SFLOAT";
		case VK_FORMAT_R16G16B16_UNORM:
			return "VK_FORMAT_R16G16B16_UNORM";
		case VK_FORMAT_R16G16B16_SNORM:
			return "VK_FORMAT_R16G16B16_SNORM";
		case VK_FORMAT_R16G16B16_USCALED:
			return "VK_FORMAT_R16G16B16_USCALED";
		case VK_FORMAT_R16G16B16_SSCALED:
			return "VK_FORMAT_R16G16B16_SSCALED";
		case VK_FORMAT_R16G16B16_UINT:
			return "VK_FORMAT_R16G16B16_UINT";
		case VK_FORMAT_R16G16B16_SINT:
			return "VK_FORMAT_R16G16B16_SINT";
		case VK_FORMAT_R16G16B16_SFLOAT:
			return "VK_FORMAT_R16G16B16_SFLOAT";
		case VK_FORMAT_R16G16B16A16_UNORM:
			return "VK_FORMAT_R16G16B16A16_UNORM";
		case VK_FORMAT_R16G16B16A16_SNORM:
			return "VK_FORMAT_R16G16B16A16_SNORM";
		case VK_FORMAT_R16G16B16A16_USCALED:
			return "VK_FORMAT_R16G16B16A16_USCALED";
		case VK_FORMAT_R16G16B16A16_SSCALED:
			return "VK_FORMAT_R16G16B16A16_SSCALED";
		case VK_FORMAT_R16G16B16A16_UINT:
			return "VK_FORMAT_R16G16B16A16_UINT";
		case VK_FORMAT_R16G16B16A16_SINT:
			return "VK_FORMAT_R16G16B16A16_SINT";
		case VK_FORMAT_R16G16B16A16_SFLOAT:
			return "VK_FORMAT_R16G16B16A16_SFLOAT";
		case VK_FORMAT_R32_UINT:
			return "VK_FORMAT_R32_UINT";
		case VK_FORMAT_R32_SINT:
			return "VK_FORMAT_R32_SINT";
		case VK_FORMAT_R32_SFLOAT:
			return "VK_FORMAT_R32_SFLOAT";
		case VK_FORMAT_R32G32_UINT:
			return "VK_FORMAT_R32G32_UINT";
		case VK_FORMAT_R32G32_SINT:
			return "VK_FORMAT_R32G32_SINT";
		case VK_FORMAT_R32G32_SFLOAT:
			return "VK_FORMAT_R32G32_SFLOAT";
		case VK_FORMAT_R32G32B32_UINT:
			return "VK_FORMAT_R32G32B32_UINT";
		case VK_FORMAT_R32G32B32_SINT:
			return "VK_FORMAT_R32G32B32_SINT";
		case VK_FORMAT_R32G32B32_SFLOAT:
			return "VK_FORMAT_R32G32B32_SFLOAT";
		case VK_FORMAT_R32G32B32A32_UINT:
			return "VK_FORMAT_R32G32B32A32_UINT";
		case VK_FORMAT_R32G32B32A32_SINT:
			return "VK_FORMAT_R32G32B32A32_SINT";
		case VK_FORMAT_R32G32B32A32_SFLOAT:
			return "VK_FORMAT_R32G32B32A32_SFLOAT";
		case VK_FORMAT_R64_UINT:
			return "VK_FORMAT_R64_UINT";
		case VK_FORMAT_R64_SINT:
			return "VK_FORMAT_R64_SINT";
		case VK_FORMAT_R64_SFLOAT:
			return "VK_FORMAT_R64_SFLOAT";
		case VK_FORMAT_R64G64_UINT:
			return "VK_FORMAT_R64G64_UINT";
		case VK_FORMAT_R64G64_SINT:
			return "VK_FORMAT_R64G64_SINT";
		case VK_FORMAT_R64G64_SFLOAT:
			return "VK_FORMAT_R64G64_SFLOAT";
		case VK_FORMAT_R64G64B64_UINT:
			return "VK_FORMAT_R64G64B64_UINT";
		case VK_FORMAT_R64G64B64_SINT:
			return "VK_FORMAT_R64G64B64_SINT";
		case VK_FORMAT_R64G64B64_SFLOAT:
			return "VK_FORMAT_R64G64B64_SFLOAT";
		case VK_FORMAT_R64G64B64A64_UINT:
			return "VK_FORMAT_R64G64B64A64_UINT";
		case VK_FORMAT_R64G64B64A64_SINT:
			return "VK_FORMAT_R64G64B64A64_SINT";
		case VK_FORMAT_R64G64B64A64_SFLOAT:
			return "VK_FORMAT_R64G64B64A64_SFLOAT";
		case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
			return "VK_FORMAT_B10G11R11_UFLOAT_PACK32";
		case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
			return "VK_FORMAT_E5B9G9R9_UFLOAT_PACK32";
		case VK_FORMAT_D16_UNORM:
			return "VK_FORMAT_D16_UNORM";
		case VK_FORMAT_X8_D24_UNORM_PACK32:
			return "VK_FORMAT_X8_D24_UNORM_PACK32";
		case VK_FORMAT_D32_SFLOAT:
			return "VK_FORMAT_D32_SFLOAT";
		case VK_FORMAT_S8_UINT:
			return "VK_FORMAT_S8_UINT";
		case VK_FORMAT_D16_UNORM_S8_UINT:
			return "VK_FORMAT_D16_UNORM_S8_UINT";
		case VK_FORMAT_D24_UNORM_S8_UINT:
			return "VK_FORMAT_D24_UNORM_S8_UINT";
		case VK_FORMAT_D32_SFLOAT_S8_UINT:
			return "VK_FORMAT_D32_SFLOAT_S8_UINT";
		case VK_FORMAT_UNDEFINED:
			return "VK_FORMAT_UNDEFINED";
		default:
			break;
	}
	return "VK_FORMAT_INVALID";
}

std::string ToString(VkColorSpaceKHR ColorSpace) {
	switch (ColorSpace) {
		case VK_COLOR_SPACE_SRGB_NONLINEAR_KHR:
			return "VK_COLOR_SPACE_SRGB_NONLINEAR_KHR";
		default:
			break;
	}
	return "VK_COLORSPACE_INVALID";
}

std::string ToString(VkPresentModeKHR present_mode) {
	switch (present_mode) {
		case VK_PRESENT_MODE_MAILBOX_KHR:
			return "VK_PRESENT_MODE_MAILBOX_KHR";
		case VK_PRESENT_MODE_IMMEDIATE_KHR:
			return "VK_PRESENT_MODE_IMMEDIATE_KHR";
		case VK_PRESENT_MODE_FIFO_KHR:
			return "VK_PRESENT_MODE_FIFO_KHR";
		case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
			return "VK_PRESENT_MODE_FIFO_RELAXED_KHR";
		case VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR:
			return "VK_PRESENT_MODE_SHARED_CONTINUOUS_REFRESH_KHR";
		case VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR:
			return "VK_PRESENT_MODE_SHARED_DEMAND_REFRESH_KHR";
		default:
			return "UNKNOWN_PRESENT_MODE";
	}
}

std::string ToString(VkSurfaceTransformFlagBitsKHR transform_flag) {
	switch (transform_flag) {
		case VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR:
			return "SURFACE_TRANSFORM_IDENTITY";
		case VK_SURFACE_TRANSFORM_ROTATE_90_BIT_KHR:
			return "SURFACE_TRANSFORM_ROTATE_90";
		case VK_SURFACE_TRANSFORM_ROTATE_180_BIT_KHR:
			return "SURFACE_TRANSFORM_ROTATE_180";
		case VK_SURFACE_TRANSFORM_ROTATE_270_BIT_KHR:
			return "SURFACE_TRANSFORM_ROTATE_270";
		case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_BIT_KHR:
			return "SURFACE_TRANSFORM_HORIZONTAL_MIRROR";
		case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90_BIT_KHR:
			return "SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90";
		case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180_BIT_KHR:
			return "SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180";
		case VK_SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270_BIT_KHR:
			return "SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270";
		case VK_SURFACE_TRANSFORM_INHERIT_BIT_KHR:
			return "SURFACE_TRANSFORM_INHERIT";
		case VK_SURFACE_TRANSFORM_FLAG_BITS_MAX_ENUM_KHR:
			return "SURFACE_TRANSFORM_FLAG_BITS_MAX_ENUM";
		default:
			break;
	}
	return "[Unknown transform flag]";
}

std::string ToString(VkSurfaceFormatKHR surface_format) {
	std::string surface_format_string = ToString(surface_format.format) + ", ";

	switch (surface_format.colorSpace) {
		case VK_COLORSPACE_SRGB_NONLINEAR_KHR:
			surface_format_string += "VK_COLORSPACE_SRGB_NONLINEAR_KHR";
			break;
		default:
			surface_format_string += "UNKNOWN COLOR SPACE";
	}
	return surface_format_string;
}

std::string ToString(VkCompositeAlphaFlagBitsKHR composite_alpha) {
	switch (composite_alpha) {
		case VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR:
			return "VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR";
		case VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR:
			return "VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR";
		case VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR:
			return "VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR";
		case VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR:
			return "VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR";
		case VK_COMPOSITE_ALPHA_FLAG_BITS_MAX_ENUM_KHR:
			return "VK_COMPOSITE_ALPHA_FLAG_BITS_MAX_ENUM_KHR";
		default:
			return "UNKNOWN COMPOSITE ALPHA FLAG";
	}
}

std::string ToString(VkImageUsageFlagBits image_usage) {
	switch (image_usage) {
		case VK_IMAGE_USAGE_TRANSFER_SRC_BIT:
			return "VK_IMAGE_USAGE_TRANSFER_SRC_BIT";
		case VK_IMAGE_USAGE_TRANSFER_DST_BIT:
			return "VK_IMAGE_USAGE_TRANSFER_DST_BIT";
		case VK_IMAGE_USAGE_SAMPLED_BIT:
			return "VK_IMAGE_USAGE_SAMPLED_BIT";
		case VK_IMAGE_USAGE_STORAGE_BIT:
			return "VK_IMAGE_USAGE_STORAGE_BIT";
		case VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT:
			return "VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT";
		case VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT:
			return "VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT";
		case VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT:
			return "VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT";
		case VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT:
			return "VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT";
		case VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM:
			return "VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM";
		default:
			return "UNKNOWN IMAGE USAGE FLAG";
	}
}

std::string ToString(bool flag) {
	if (flag == true) {
		return "true";
	}
	return "false";
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugUtilsMessengerCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type,
	const VkDebugUtilsMessengerCallbackDataEXT * callback_data, void * user_data
) {
	// Log debug messge
	if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		X_DEBUG_PRINTF(
			"DebugUtilMessageCallback(Warning): MessageId=%" PRIi32 ", MessageIdName=%s, Message=%s", callback_data->messageIdNumber,
			callback_data->pMessageIdName, callback_data->pMessage
		);
	} else if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
		X_DEBUG_PRINTF(
			"DebugUtilMessageCallback: MessageId=%" PRIi32 ", MessageIdName=\"%s\", Message=%s", callback_data->messageIdNumber,
			callback_data->pMessageIdName, callback_data->pMessage
		);
	}
	return VK_FALSE;
}

VKAPI_ATTR VkBool32 VKAPI_CALL DebugReportCallback(
	VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT /*type*/, uint64_t /*object*/, size_t /*location*/, int32_t /*message_code*/,
	const char * layer_prefix, const char * message, void * /*user_data*/
) {
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		X_DEBUG_PRINTF("DebugReportCallback: %s, %s", layer_prefix, message);
	} else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
		X_DEBUG_PRINTF("DebugReportCallback(Warngin): %s, %s", layer_prefix, message);
	} else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
		X_DEBUG_PRINTF("DebugReportCallback(Warngin): %s, %s", layer_prefix, message);
	} else {
		X_DEBUG_PRINTF("DebugReportCallback: %s, %s", layer_prefix, message);
	}
	return VK_FALSE;
}

static bool ValidateLayers(const std::vector<const char *> & required, const std::vector<VkLayerProperties> & available) {
	for (auto layer : required) {
		bool found = false;
		for (auto & available_layer : available) {
			if (strcmp(available_layer.layerName, layer) == 0) {
				found = true;
				break;
			}
		}
		if (!found) {
			X_DEBUG_PRINTF("ValidationLayers: Layer:%s not found", layer);
			return false;
		}
	}
	return true;
}

std::vector<const char *> GetOptimalValidationLayers(const std::vector<VkLayerProperties> & supported_instance_layers) {
	std::vector<std::vector<const char *>> validation_layer_priority_list = {
		// The preferred validation layer is "VK_LAYER_KHRONOS_validation"
		{ "VK_LAYER_KHRONOS_validation" },
		// Otherwise we fallback to using the LunarG meta layer
		{ "VK_LAYER_LUNARG_standard_validation" },
		// Otherwise we attempt to enable the individual layers that compose the LunarG meta layer since it doesn't exist
		{
			"VK_LAYER_GOOGLE_threading",
			"VK_LAYER_LUNARG_parameter_validation",
			"VK_LAYER_LUNARG_object_tracker",
			"VK_LAYER_LUNARG_core_validation",
			"VK_LAYER_GOOGLE_unique_objects",
		},
		// Otherwise as a last resort we fallback to attempting to enable the LunarG core layer
		{ "VK_LAYER_LUNARG_core_validation" }
	};

	for (auto & validation_layers : validation_layer_priority_list) {
		if (ValidateLayers(validation_layers, supported_instance_layers)) {
			for (auto & layer : validation_layers) {
				X_DEBUG_PRINTF("EnableValidationLayer: %s", layer);
			}
			return validation_layers;
		}
	}

	// Else return nothing
	X_DEBUG_PRINTF("GetOptionalValidationLayers: Couldn't enable validation layers - falling back");
	return {};
}
X_END
