#ifndef CG_SEM5_VKASSERT_H
#define CG_SEM5_VKASSERT_H

#include <stdexcept>
#include <string_view>
#include <vulkan/vulkan.h>

void vk_assert(VkResult, std::string_view message);

#endif // CG_SEM5_VKASSERT_H