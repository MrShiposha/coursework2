#include "vkassert.h"

void vk_assert(VkResult result, std::string_view message)
{
    if(result != VK_SUCCESS)
        throw std::runtime_error(message.data());
}