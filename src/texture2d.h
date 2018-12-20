#ifndef CG_SEM5_TEXTURE2D_H
#define CG_SEM5_TEXTURE2D_H

#include <string_view>

#include "texture.h"

struct Texture2D : public Texture
{
    static std::shared_ptr<Texture2D> load_from_file
    (
        std::string_view path,
        VkFormat format,
        std::shared_ptr<Device> device,
        VkCommandPool command_pool,
        VkQueue copy_queue,
        VkImageUsageFlags image_usage_flags = VK_IMAGE_USAGE_SAMPLED_BIT,
        VkImageLayout image_layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    );
};

#endif // CG_SEM5_TEXTURE2D_H