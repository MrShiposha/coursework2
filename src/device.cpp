#include <algorithm>
#include <fstream>

#include "device.h"
#include "vkassert.h"

Device::Device(VkPhysicalDevice physical_device)
: physical_device(physical_device)
{
    assert(physical_device != nullptr);

    vkGetPhysicalDeviceProperties(physical_device, &properties);
    vkGetPhysicalDeviceFeatures(physical_device, &features);
    vkGetPhysicalDeviceMemoryProperties(physical_device, &memory_properties);

    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);
    assert(queue_family_count > 0);

    queue_family_properties.resize(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_family_properties.data());

    uint32_t extension_count = 0;
    vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, nullptr);
    if(extension_count > 0)
    {
        std::vector<VkExtensionProperties> extensions(extension_count);
        if(vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, extensions.data()) == VK_SUCCESS)
            for(auto &&extension : extensions)
                supported_extensions.push_back(extension.extensionName);
    }
}

Device::~Device()
{
    if(logical_device)
        vkDestroyDevice(logical_device, nullptr);
}

Device::operator VkPhysicalDevice() const
{
    return physical_device;
}

Device::operator VkDevice() const
{
    return logical_device;
}


std::optional<MemoryTypeIndex> Device::find_memory_type(uint32_t type_bits, VkMemoryPropertyFlags properties)
{
    for(uint32_t i = 0; i < memory_properties.memoryTypeCount; ++i)
    {
        if((type_bits & 1) == 1)
        {
            if((memory_properties.memoryTypes[i].propertyFlags & properties) == properties)
                return i;
        }

        type_bits >>= 1;
    }

    return {};
}


QueueFamilyIndex Device::find_queue_family_index(VkQueueFlagBits queue_flags)
{
    if(queue_flags & VK_QUEUE_COMPUTE_BIT)
        for(uint32_t i = 0; i < static_cast<uint32_t>(queue_family_properties.size()); ++i)
        {
            if((queue_family_properties[i].queueFlags & queue_flags) && ((queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
                return i;
        }
    
    if(queue_flags & VK_QUEUE_TRANSFER_BIT)
    {
        for(uint32_t i = 0; i < static_cast<uint32_t>(queue_family_properties.size()); ++i)
        {
            if
            (
                (queue_family_properties[i].queueFlags & queue_flags) 
                && ((queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0)
                && ((queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0)
            ) return i;
        }
    }

    for(uint32_t i = 0; i < static_cast<uint32_t>(queue_family_properties.size()); ++i)
        if(queue_family_properties[i].queueFlags & queue_flags)
            return i;

    throw std::runtime_error("Can't find queue family index");    
}


VkResult Device::initialize_logical_device
(
    VkPhysicalDeviceFeatures enabled_features, 
    const std::vector<const char*> &enabled_extensions,
    VkQueueFlags requested_queue_types
)
{
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;
    auto is_create_info_unique = [&](QueueFamilyIndex index)
    {
        return std::find_if
        (
            queue_create_infos.begin(),
            queue_create_infos.end(),
            [&index](const auto &create_info)
            {
                return create_info.queueFamilyIndex == index;
            }
        ) == queue_create_infos.end();
    };

    const float default_queue_priority = 0.f;

    if(requested_queue_types & VK_QUEUE_GRAPHICS_BIT)
    {
        queue_family_indices.graphics = find_queue_family_index(VK_QUEUE_GRAPHICS_BIT);
        VkDeviceQueueCreateInfo queue_info = {};
        queue_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info.queueFamilyIndex        = queue_family_indices.graphics;
        queue_info.queueCount              = 1;
        queue_info.pQueuePriorities        = &default_queue_priority;
        queue_create_infos.push_back(queue_info);
    }
    else
        queue_family_indices.graphics = NO_INDEX;

    if(requested_queue_types & VK_QUEUE_COMPUTE_BIT)
    {
        queue_family_indices.compute = find_queue_family_index(VK_QUEUE_COMPUTE_BIT);
        VkDeviceQueueCreateInfo queue_info = {};
        queue_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info.queueFamilyIndex        = queue_family_indices.compute;
        queue_info.queueCount              = 1;
        queue_info.pQueuePriorities        = &default_queue_priority;

        // Ensure unique create infos
        if(is_create_info_unique(queue_family_indices.compute)) 
            queue_create_infos.push_back(queue_info);
    }
    else
        queue_family_indices.compute = queue_family_indices.graphics;
    
    if(requested_queue_types & VK_QUEUE_TRANSFER_BIT)
    {
        queue_family_indices.transfer = find_queue_family_index(VK_QUEUE_TRANSFER_BIT);
        VkDeviceQueueCreateInfo queue_info = {};
        queue_info.sType                   = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info.queueFamilyIndex        = queue_family_indices.transfer;
        queue_info.queueCount              = 1;
        queue_info.pQueuePriorities        = &default_queue_priority;

        if(is_create_info_unique(queue_family_indices.transfer))
            queue_create_infos.push_back(queue_info);
    }
    else
        queue_family_indices.transfer = queue_family_indices.graphics;

    VkDeviceCreateInfo device_create_info   = {};
    device_create_info.sType                = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
    device_create_info.pQueueCreateInfos    = queue_create_infos.data();
    device_create_info.pEnabledFeatures     = &enabled_features;

    if(enabled_extensions.size() > 0)
    {
        device_create_info.enabledExtensionCount   = static_cast<uint32_t>(enabled_extensions.size());
        device_create_info.ppEnabledExtensionNames = enabled_extensions.data();
    }

    this->enabled_features = enabled_features;

    return vkCreateDevice(physical_device, &device_create_info, nullptr, &logical_device);
}

VkResult Device::create_buffer
(
    VkBufferUsageFlags usage_flags,
    VkMemoryPropertyFlags memory_property_flags,
    VkBuffer *buffer,
    VkDeviceSize buffer_size,
    VkDeviceMemory *buffer_memory,
    const void *buffer_data
)
{
    VkBufferCreateInfo buffer_create_info = {};
    buffer_create_info.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.usage              = usage_flags;
    buffer_create_info.size               = buffer_size;
    buffer_create_info.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;
    
    vk_assert
    (
        vkCreateBuffer(logical_device, &buffer_create_info, nullptr, buffer),
        "Can't create buffer"
    );

    VkMemoryRequirements memory_reqs;
    vkGetBufferMemoryRequirements(logical_device, *buffer, &memory_reqs);
    
    VkMemoryAllocateInfo allocate_info = {};
    allocate_info.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = memory_reqs.size;
    allocate_info.memoryTypeIndex = find_memory_type(memory_reqs.memoryTypeBits, memory_property_flags).value();

    vk_assert
    (
        vkAllocateMemory(logical_device, &allocate_info, nullptr, buffer_memory),
        "Can't allocate memory for buffer"
    );

    if(buffer_data != nullptr)
    {
        void *mapped_data;
        vk_assert
        (
            vkMapMemory(logical_device, *buffer_memory, 0, buffer_size, 0, &mapped_data),
            "Can't map buffer memory"
        );

        std::memcpy(mapped_data, buffer_data, buffer_size);

        if((memory_property_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
        {
            VkMappedMemoryRange mapped_range = {};
            mapped_range.sType               = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
            mapped_range.memory              = *buffer_memory;
            mapped_range.offset              = 0;
            mapped_range.size                = buffer_size;
            vkFlushMappedMemoryRanges(logical_device, 1, &mapped_range);
        }

        vkUnmapMemory(logical_device, *buffer_memory);
    }

    vk_assert
    (
        vkBindBufferMemory(logical_device, *buffer, *buffer_memory, 0),
        "Can't bind buffer memory"
    );

    return VK_SUCCESS;
}


VkResult Device::create_buffer
(
    VkBufferUsageFlags usage_flags,
    VkMemoryPropertyFlags memory_property_flags,
    std::shared_ptr<DeviceBuffer> device_buffer,
    VkDeviceSize size, 
    void *buffer_data
)
{
    device_buffer->device = logical_device;

    VkBufferCreateInfo buffer_create_info = {};
    buffer_create_info.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buffer_create_info.usage              = usage_flags;
    buffer_create_info.size               = size;
    buffer_create_info.sharingMode        = VK_SHARING_MODE_EXCLUSIVE;
    
    vk_assert
    (
        vkCreateBuffer(logical_device, &buffer_create_info, nullptr, &device_buffer->buffer),
        "Can't create buffer"
    );

    VkMemoryRequirements memory_reqs;
    vkGetBufferMemoryRequirements(logical_device, *device_buffer, &memory_reqs);
    
    VkMemoryAllocateInfo allocate_info = {};
    allocate_info.sType          = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocate_info.allocationSize = memory_reqs.size;
    allocate_info.memoryTypeIndex = find_memory_type(memory_reqs.memoryTypeBits, memory_property_flags).value();

    vk_assert
    (
        vkAllocateMemory(logical_device, &allocate_info, nullptr, &device_buffer->memory),
        "Can't allocate memory for buffer"
    );

    device_buffer->alignment             = memory_reqs.alignment;
    device_buffer->size                  = allocate_info.allocationSize;
    device_buffer->usage_flags           = usage_flags;
    device_buffer->memory_property_flags = memory_property_flags;

    if(buffer_data != nullptr)
    {
        vk_assert
        (
            device_buffer->map(),
            "Can't map buffer memory"
        );

        std::memcpy(device_buffer->mapped_memory, buffer_data, device_buffer->size);
        device_buffer->unmap();
    }

    device_buffer->setup_descriptor();
    return device_buffer->bind_memory();
}


void Device::copy_buffer_data
(
    std::shared_ptr<DeviceBuffer> src,
    std::shared_ptr<DeviceBuffer> dest,
    VkCommandPool command_pool,
    VkQueue copy_queue,
    std::optional<VkBufferCopy> copy_region
)
{
    assert(src->size <= dest->size);
    assert(src->buffer);

    VkCommandBuffer copy_cmd = create_command_buffer(command_pool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);
    
    VkCommandBufferBeginInfo begin_cmd = {};
    begin_cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vk_assert
    (
        vkBeginCommandBuffer(copy_cmd, &begin_cmd),
        "Can't begin buffer copy command"
    );

    VkBufferCopy buffer_copy = { 0 };
    if(copy_region)
        buffer_copy = copy_region.value();
    else
        buffer_copy.size = src->size;

    vkCmdCopyBuffer(copy_cmd, src->buffer, dest->buffer, 1, &buffer_copy);
    vk_assert
    (
        vkEndCommandBuffer(copy_cmd),
        "Can't finish command buffer"
    );

    flush_command_buffer(copy_cmd, copy_queue);
    vkFreeCommandBuffers(logical_device, command_pool, 1, &copy_cmd);
}

VkCommandPool Device::create_command_pool(QueueFamilyIndex queue_family_index, VkCommandPoolCreateFlags flags)
{
    VkCommandPoolCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.queueFamilyIndex = queue_family_index;
    create_info.flags = flags;
    
    VkCommandPool cmd_pool;
    vk_assert
    (
        vkCreateCommandPool(logical_device, &create_info, nullptr, &cmd_pool),
        "Can't create command pool"
    );

    return cmd_pool;
}


VkCommandBuffer Device::create_command_buffer(VkCommandPool command_pool, VkCommandBufferLevel level)
{
    VkCommandBufferAllocateInfo allocate_info = {};
    allocate_info.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.commandPool                 = command_pool;
    allocate_info.level                       = level;
    allocate_info.commandBufferCount          = 1;

    VkCommandBuffer cmd_buffer;
    vk_assert
    (
        vkAllocateCommandBuffers(logical_device, &allocate_info, &cmd_buffer),
        "Can't allocate command buffer"
    );

    return cmd_buffer;
}

void Device::begin_command_buffer(VkCommandBuffer command_buffer)
{
    VkCommandBufferBeginInfo begin_cmd = {};
    begin_cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    vk_assert
    (
        vkBeginCommandBuffer(command_buffer, &begin_cmd),
        "Can't begin command buffer"
    );
}

void Device::end_command_buffer(VkCommandBuffer command_buffer)
{
    vk_assert
    (
        vkEndCommandBuffer(command_buffer),
        "Can't finish command buffer"
    );
}

void Device::flush_command_buffer(VkCommandBuffer command_buffer, VkQueue queue)
{
    if(command_buffer == VK_NULL_HANDLE)
        return;

    VkSubmitInfo submit_info       = {};
    submit_info.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers    = &command_buffer;

    VkFenceCreateInfo fence_info = {};
    fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fence_info.flags = 0;

    VkFence fence;
    vk_assert
    (
        vkCreateFence(logical_device, &fence_info, nullptr, &fence),
        "Can't create fence for flush command buffer"
    );

    vk_assert
    (
        vkQueueSubmit(queue, 1, &submit_info, fence),
        "Can't submit info to queue"
    );

    vk_assert
    (
        vkWaitForFences(logical_device, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT),
        "Can't wait for fence"
    );

    vkDestroyFence(logical_device, fence, nullptr);
}

VkPipelineShaderStageCreateInfo Device::load_shader(std::string_view path, VkShaderStageFlagBits stage)
{
    using namespace std::string_literals;

    VkPipelineShaderStageCreateInfo shader_stage = {};
	shader_stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shader_stage.stage = stage;
    shader_stage.pName = "main";

    std::ifstream in(path.data(), std::ios::binary | std::ios::in | std::ios::ate);
    if(!in)
        throw std::runtime_error("Can't load shader \""s + path.data() + "\"");

    size_t size = in.tellg();
    assert(size > 0 && "Empty shader file");
    in.seekg(0, std::ios::beg);
    
    std::unique_ptr<char[]> shader_code(new char[size]);
    
    in.read(shader_code.get(), size);
    in.close();

    VkShaderModule shader_module;
    VkShaderModuleCreateInfo module_create_info = {};
    module_create_info.sType                    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    module_create_info.codeSize                 = size;
    module_create_info.pCode                    = reinterpret_cast<uint32_t*>(shader_code.get());

    vk_assert
    (
        vkCreateShaderModule(logical_device, &module_create_info, nullptr, &shader_module),
        "Can't create shader module"
    );

    shader_stage.module = shader_module;

    assert(shader_stage.module != VK_NULL_HANDLE);
    return shader_stage;
}

VkFormat Device::get_supported_depth_format()
{
    std::vector<VkFormat> depth_formats = 
    {
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM
    };

    for(auto &&format : depth_formats)
    {
        VkFormatProperties format_properties;
        vkGetPhysicalDeviceFormatProperties(physical_device, format, &format_properties);

        if(format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
            return format;
    }

    throw std::runtime_error("No depth format");
}

bool Device::is_extension_supported(std::string_view extension) const
{
    return std::find
    (
        supported_extensions.begin(),
        supported_extensions.end(),
        extension.data()
    ) != supported_extensions.end();
}
