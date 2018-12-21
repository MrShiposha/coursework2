#include <gli/gli.hpp>

#include "texture2d.h"
#include "vkassert.h"

void set_image_layout
(
    VkCommandBuffer command_buffer,
    VkImage image,
    VkImageLayout old_image_layout,
    VkImageLayout new_image_layout,
    VkImageSubresourceRange subresource_range,
    VkPipelineStageFlags src_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
    VkPipelineStageFlags dst_stage_mask = VK_PIPELINE_STAGE_ALL_COMMANDS_BIT
)
{
    // Create an image barrier object
    VkImageMemoryBarrier image_memory_barrier = {};
    image_memory_barrier.sType                = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    image_memory_barrier.srcQueueFamilyIndex  = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.dstQueueFamilyIndex  = VK_QUEUE_FAMILY_IGNORED;
    image_memory_barrier.oldLayout            = old_image_layout;
    image_memory_barrier.newLayout            = new_image_layout;
    image_memory_barrier.image                = image;
    image_memory_barrier.subresourceRange     = subresource_range;

    switch (old_image_layout)
    {
    case VK_IMAGE_LAYOUT_UNDEFINED:
        // Image layout is undefined (or does not matter)
        // Only valid as initial layout
        // No flags required, listed only for completeness
        image_memory_barrier.srcAccessMask = 0;
        break;

    case VK_IMAGE_LAYOUT_PREINITIALIZED:
        // Image is preinitialized
        // Only valid as initial layout for linear images, preserves memory contents
        // Make sure host writes have been finished
        image_memory_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        // Image is a color attachment
        // Make sure any writes to the color buffer have been finished
        image_memory_barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        // Image is a depth/stencil attachment
        // Make sure any writes to the depth/stencil buffer have been finished
        image_memory_barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        // Image is a transfer source 
        // Make sure any reads from the image have been finished
        image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        // Image is a transfer destination
        // Make sure any writes to the image have been finished
        image_memory_barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        // Image is read by a shader
        // Make sure any shader reads from the image have been finished
        image_memory_barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;
    default:
        // Other source layouts aren't handled (yet)
        break;
    }

    // Target layouts (new)
    // Destination access mask controls the dependency for the new image layout
    switch (new_image_layout)
    {
    case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
        // Image will be used as a transfer destination
        // Make sure any writes to the image have been finished
        image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
        // Image will be used as a transfer source
        // Make sure any reads from the image have been finished
        image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        break;

    case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
        // Image will be used as a color attachment
        // Make sure any writes to the color buffer have been finished
        image_memory_barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
        // Image layout will be used as a depth/stencil attachment
        // Make sure any writes to depth/stencil buffer have been finished
        image_memory_barrier.dstAccessMask = image_memory_barrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        // Image will be read in a shader (sampler, input attachment)
        // Make sure any writes to the image have been finished
        if (image_memory_barrier.srcAccessMask == 0)
        {
            image_memory_barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
        }
        image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        break;
    default:
        // Other source layouts aren't handled (yet)
        break;
    }

    // Put barrier inside setup command buffer
    vkCmdPipelineBarrier
    (
        command_buffer,
        src_stage_mask,
        dst_stage_mask,
        0,
        0, 
        nullptr,
        0, 
        nullptr,
        1, &image_memory_barrier
    );
}

std::shared_ptr<Texture2D> Texture2D::load_from_file
(
    std::string_view path,
    VkFormat format,
    std::shared_ptr<Device> device,
    VkCommandPool command_pool,
    VkQueue copy_queue,
    VkImageUsageFlags image_usage_flags,
    VkImageLayout image_layout
)
{
    auto texture = std::make_shared<Texture2D>();

    gli::texture2d texture2d(gli::load(path.data()));

    assert(!texture2d.empty() && "Empty texture");

    texture->device     = device;
    texture->width      = static_cast<uint32_t>(texture2d[0].extent().x);
    texture->height     = static_cast<uint32_t>(texture2d[0].extent().y);
    texture->mip_levels = static_cast<uint32_t>(texture2d.levels());

    VkFormatProperties format_properties;
    vkGetPhysicalDeviceFormatProperties(*device, format, &format_properties);

    VkBuffer       staging_buffer;
    VkDeviceMemory staging_memory;

    device->create_buffer
    (
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        &staging_buffer,
        texture2d.size(),
        &staging_memory,
        texture2d.data()
    );

    std::vector<VkBufferImageCopy> buffer_copy_regions;
    uint32_t offset = 0;

    for(uint32_t i = 0; i < texture->mip_levels; ++i)
    {
        VkBufferImageCopy buffer_copy_region               = {};
        buffer_copy_region.imageSubresource.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        buffer_copy_region.imageSubresource.mipLevel       = i;
        buffer_copy_region.imageSubresource.baseArrayLayer = 0;
        buffer_copy_region.imageSubresource.layerCount     = 1;
        buffer_copy_region.imageExtent.width               = static_cast<uint32_t>(texture2d[i].extent().x);
        buffer_copy_region.imageExtent.height              = static_cast<uint32_t>(texture2d[i].extent().y);
        buffer_copy_region.imageExtent.depth               = 1;
        buffer_copy_region.bufferOffset                    = offset;

        buffer_copy_regions.push_back(buffer_copy_region);

        offset += static_cast<uint32_t>(texture2d[i].size());
    }


    // Create optimal tiled target image
    VkImageCreateInfo image_create_info = {};
    image_create_info.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.imageType         = VK_IMAGE_TYPE_2D;
    image_create_info.format            = format;
    image_create_info.mipLevels         = texture->mip_levels;
    image_create_info.arrayLayers       = 1;
    image_create_info.samples           = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling            = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.sharingMode       = VK_SHARING_MODE_EXCLUSIVE;
    image_create_info.initialLayout     = VK_IMAGE_LAYOUT_UNDEFINED;
    image_create_info.extent            = { texture->width, texture->height, 1 };
    image_create_info.usage             = image_usage_flags;

    // Ensure that the TRANSFER_DST_BIT is set for staging
    if((image_create_info.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT) == 0)
        image_create_info.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    vk_assert
    (
        vkCreateImage(*device, &image_create_info, nullptr, &texture->image),
        "Can't create image"
    );

    VkMemoryRequirements memory_reqs;
    vkGetImageMemoryRequirements(*device, texture->image, &memory_reqs);

    VkMemoryAllocateInfo allocate_info = {};
    allocate_info.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize       = memory_reqs.size;
    allocate_info.memoryTypeIndex      = device->find_memory_type(memory_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT).value();
    vk_assert
    (
        vkAllocateMemory(*device, &allocate_info, nullptr, &texture->device_memory),
        "Can't allocate texture memory"
    );

    vk_assert
    (
        vkBindImageMemory(*device, texture->image, texture->device_memory, 0),
        "Can't bind texture memory"
    );

    VkImageSubresourceRange subresource_range = {};
    subresource_range.aspectMask              = VK_IMAGE_ASPECT_COLOR_BIT;
    subresource_range.baseMipLevel            = 0;
    subresource_range.levelCount              = texture->mip_levels;
    subresource_range.layerCount              = 1;

    VkCommandBuffer copy_cmd = device->create_command_buffer(command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    device->begin_command_buffer(copy_cmd);

    set_image_layout
    (
        copy_cmd,
        texture->image,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        subresource_range
    );

    vkCmdCopyBufferToImage
    (
        copy_cmd,
        staging_buffer,
        texture->image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        static_cast<uint32_t>(buffer_copy_regions.size()),
        buffer_copy_regions.data()
    );

    texture->image_layout = image_layout;
    set_image_layout
    (
        copy_cmd,
        texture->image,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        image_layout,
        subresource_range
    );

    device->end_command_buffer(copy_cmd);
    device->flush_command_buffer(copy_cmd, copy_queue);

    vkFreeMemory(*device, staging_memory, nullptr);
    vkDestroyBuffer(*device, staging_buffer, nullptr);

    // Create default sampler
    VkSamplerCreateInfo sampler_create_info = {};
    sampler_create_info.sType               = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_create_info.magFilter           = VK_FILTER_LINEAR;
    sampler_create_info.minFilter           = VK_FILTER_LINEAR;
    sampler_create_info.mipmapMode          = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    sampler_create_info.addressModeU        = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeV        = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.addressModeW        = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_create_info.mipLodBias          = 0.f;
    sampler_create_info.compareOp           = VK_COMPARE_OP_NEVER;
    sampler_create_info.minLod              = 0.f;
    sampler_create_info.maxLod              = texture->mip_levels; // Max level of detail must match mip level count
    sampler_create_info.maxAnisotropy       = device->enabled_features.samplerAnisotropy? 
                                              device->properties.limits.maxSamplerAnisotropy : 1.f;
    sampler_create_info.anisotropyEnable    = device->enabled_features.samplerAnisotropy;
    sampler_create_info.borderColor         = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    vk_assert
    (
        vkCreateSampler(*device, &sampler_create_info, nullptr, &texture->sampler),
        "Can't craete texture sampler"
    );

    VkImageViewCreateInfo view_create_info = {};
    view_create_info.sType                 = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_create_info.viewType              = VK_IMAGE_VIEW_TYPE_2D;
    view_create_info.format                = format;
    view_create_info.components            = 
    { 
        VK_COMPONENT_SWIZZLE_R, 
        VK_COMPONENT_SWIZZLE_G, 
        VK_COMPONENT_SWIZZLE_B, 
        VK_COMPONENT_SWIZZLE_A 
    };
    view_create_info.subresourceRange            = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
    view_create_info.subresourceRange.levelCount = texture->mip_levels;
    view_create_info.image                       = texture->image;
    vk_assert
    (
        vkCreateImageView(*device, &view_create_info, nullptr, &texture->view),
        "Can't create image view for texture"
    );

    texture->update_descriptor();

    return texture;
}