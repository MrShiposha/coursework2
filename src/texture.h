#ifndef CG_SEM5_COURSEWORK_TEXTURE_H
#define CG_SEM5_COURSEWORK_TEXTURE_H

#include <memory>
#include <vulkan/vulkan.h>

#include "device.h"

struct Texture 
{
    std::shared_ptr<Device> device;
    VkImage                 image;
    VkImageLayout           image_layout;
    VkDeviceMemory          device_memory;
    VkImageView             view;
    uint32_t                width, height;
    uint32_t                mip_levels;
    uint32_t                layer_count;
    VkDescriptorImageInfo   descriptor;

    VkSampler sampler;

    virtual ~Texture();

    void update_descriptor();

    void destroy();
};

#endif // CG_SEM5_COURSEWORK_TEXTURE_H