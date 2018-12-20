#include <memory>

#include "devicebuffer.h"

DeviceBuffer::~DeviceBuffer()
{
    destroy();
}

DeviceBuffer::operator VkBuffer() const
{
    return buffer;
}

VkResult DeviceBuffer::map(VkDeviceSize size, VkDeviceSize offset)
{
    return vkMapMemory(device, memory, offset, size, 0, &mapped_memory);
}

void DeviceBuffer::unmap()
{
    if(mapped_memory != nullptr)
    {
        vkUnmapMemory(device, memory);
        mapped_memory = nullptr;
    }
}

VkResult DeviceBuffer::bind_memory(VkDeviceSize offset)
{
    return vkBindBufferMemory(device, buffer, memory, offset);
}

void DeviceBuffer::setup_descriptor(VkDeviceSize size, VkDeviceSize offset)
{
    descriptor.offset = offset;
    descriptor.buffer = buffer;
    descriptor.range  = size;
}

void DeviceBuffer::fill(void *src, VkDeviceSize size) const
{
    if(mapped_memory == nullptr)
        throw std::runtime_error("Device buffer is not mapped to host memory. Can't copy data");

    std::memcpy(mapped_memory, src, size);
}

VkResult DeviceBuffer::flush(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange mapped_range = {};
    mapped_range.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mapped_range.memory = memory;
    mapped_range.offset = offset;
    mapped_range.size   = size;
    return vkFlushMappedMemoryRanges(device, 1, &mapped_range);
}

VkResult DeviceBuffer::make_host_visible(VkDeviceSize size, VkDeviceSize offset)
{
    VkMappedMemoryRange mapped_range = {};
    mapped_range.sType  = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
    mapped_range.memory = memory;
    mapped_range.offset = offset;
    mapped_range.size   = size;
    return vkInvalidateMappedMemoryRanges(device, 1, &mapped_range);
}

void DeviceBuffer::destroy()
{
    if(buffer)
        vkDestroyBuffer(device, buffer, nullptr);
    
    if(memory)
        vkFreeMemory(device, memory, nullptr);
}