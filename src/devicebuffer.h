#ifndef CG_SEM5_DEVICEBUFFER_H
#define CG_SEM5_DEVICEBUFFER_H

#include <stdexcept>
#include <vulkan/vulkan.h>

struct DeviceBuffer
{
    VkDevice               device     = VK_NULL_HANDLE;
    VkBuffer               buffer     = VK_NULL_HANDLE;
    VkDeviceMemory         memory     = VK_NULL_HANDLE;
    VkDescriptorBufferInfo descriptor = { 0 };
    VkDeviceSize           size       = 0;
    VkDeviceSize           alignment  = 0;
    void *mapped_memory = nullptr;

    VkBufferUsageFlags    usage_flags           = 0;
    VkMemoryPropertyFlags memory_property_flags = 0;

    ~DeviceBuffer();

    operator VkBuffer() const;

    VkResult map(VkDeviceSize = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    void unmap();

    VkResult bind_memory(VkDeviceSize offset = 0);

    void setup_descriptor(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    void fill(void *src, VkDeviceSize size) const;

    VkResult flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    VkResult make_host_visible(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

    void destroy();
};

#endif // CG_SEM5_DEVICEBUFFER_H