#include "./vk.hpp"

#include "../xengine/engine.hpp"
#include "./vk_debug.hpp"

X_BEGIN

vkb::Instance VulkanInstance = {};  // Vulkan library handle

bool InitVulkan() {

	auto InstanceGuard = xValueGuard(VulkanInstance);

	vkb::InstanceBuilder builder;
	// make the vulkan instance, with basic debug features
	auto inst_ret = builder
						.set_app_name("XEngine Vulkan Application")
#ifndef NDEBUG
						.request_validation_layers(true)
						.use_default_debug_messenger()
#endif
						.require_api_version(1, 3, 0)
						.build();

	if (!inst_ret.has_value()) {
		X_DEBUG_PRINTF("error: %s", ToString(inst_ret.vk_result()).c_str());
		return false;
	}
	VulkanInstance = inst_ret.value();

	InstanceGuard.Dismiss();
	return true;
}

void CleanVulkan() {
	vkb::destroy_instance(Steal(VulkanInstance));
}

X_END
