#ifndef CG_SEM5_DEVICE_H
#define CG_SEM5_DEVICE_H

#include <limits>
#include <string>
#include <vector>
#include <optional>
#include <stdexcept>
#include <vulkan/vulkan.h>

#include "devicebuffer.h"
#include "vkdef.h"

using QueueFamilyIndex = uint32_t;
using MemoryTypeIndex  = uint32_t;

static constexpr QueueFamilyIndex NO_INDEX = std::numeric_limits<QueueFamilyIndex>::max();

struct Device
{
    VkPhysicalDevice                     physical_device;
    VkDevice                             logical_device;
    VkPhysicalDeviceProperties           properties;
    VkPhysicalDeviceFeatures             features;
    VkPhysicalDeviceFeatures             enabled_features;
    VkPhysicalDeviceMemoryProperties     memory_properties;
    std::vector<VkQueueFamilyProperties> queue_family_properties;
    std::vector<std::string>             supported_extensions;

    struct 
    {
        QueueFamilyIndex graphics;
        QueueFamilyIndex compute;
        QueueFamilyIndex transfer;
    } queue_family_indices;

    Device(VkPhysicalDevice);
    ~Device();

    operator VkPhysicalDevice() const;

    operator VkDevice() const;

    std::optional<MemoryTypeIndex> find_memory_type(uint32_t type_bits, VkMemoryPropertyFlags properties);

    QueueFamilyIndex find_queue_family_index(VkQueueFlagBits queue_flags);

    VkResult initialize_logical_device
    (
        VkPhysicalDeviceFeatures enabled_features, 
        const std::vector<const char*> &enabled_extensions,
        VkQueueFlags requested_queue_types = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT
    );

    VkResult create_buffer
    (
        VkBufferUsageFlags usage_flags,
        VkMemoryPropertyFlags memory_property_flags,
        VkBuffer *,
        VkDeviceSize buffer_size,
        VkDeviceMemory *buffer_memory,
        const void *buffer_data = nullptr
    );

    VkResult create_buffer
    (
        VkBufferUsageFlags usage_flags,
        VkMemoryPropertyFlags memory_property_flags,
        std::shared_ptr<DeviceBuffer> device_buffer,
        VkDeviceSize size,
        void *buffer_data = nullptr
    );

    void copy_buffer_data
    (
        std::shared_ptr<DeviceBuffer> src,
        std::shared_ptr<DeviceBuffer> dest,
        VkCommandPool command_pool,
        VkQueue copy_queue,
        std::optional<VkBufferCopy> copy_region
    );

    VkCommandPool create_command_pool(QueueFamilyIndex queue_family_index, VkCommandPoolCreateFlags);

    VkCommandBuffer create_command_buffer(VkCommandPool command_pool, VkCommandBufferLevel level);

    void begin_command_buffer(VkCommandBuffer command_buffer);

    void end_command_buffer(VkCommandBuffer command_buffer);

    void flush_command_buffer(VkCommandBuffer command_buffer, VkQueue queue);

    VkPipelineShaderStageCreateInfo load_shader(std::string_view path, VkShaderStageFlagBits stage);

    VkFormat get_supported_depth_format();

    bool is_extension_supported(std::string_view extension) const;
};

#endif // CG_SEM5_DEVICE_H