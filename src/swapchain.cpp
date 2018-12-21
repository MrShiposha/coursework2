#include <limits>

#include "swapchain.h"
#include "vkassert.h"

Swapchain::Swapchain()
: swapchain(VK_NULL_HANDLE)
{}

void Swapchain::initialize_surface(const Window &window)
{
    VkResult result = VK_SUCCESS;

#if defined(VK_USE_PLATFORM_MACOS_MVK)
    VkMacOSSurfaceCreateInfoMVK surface_create_info = {};
    surface_create_info.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
    surface_create_info.pNext = NULL;
    surface_create_info.flags = 0;
    surface_create_info.pView = window.get_view();
    result = vkCreateMacOSSurfaceMVK(instance, &surface_create_info, NULL, &surface);
#endif 

    if(result != VK_SUCCESS)
        throw std::runtime_error("Could not initialize surface");

    uint32_t queue_count;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count, NULL);
    if(queue_count < 1)
        throw std::runtime_error("No queue families");

    std::vector<VkQueueFamilyProperties> queue_properties(queue_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_count, queue_properties.data());

    std::vector<VkBool32> supports_present(queue_count);
    for (uint32_t i = 0; i < queue_count; i++) 
    {
        fpGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &supports_present[i]);
    }

    // Search for a graphics and a present queue in the array of queue
    // families, try to find one that supports both
    uint32_t graphics_queue_node_index = std::numeric_limits<uint32_t>::max();
    uint32_t present_queue_node_index  = std::numeric_limits<uint32_t>::max();
    for (uint32_t i = 0; i < queue_count; i++) 
    {
        if ((queue_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0) 
        {
            if (graphics_queue_node_index == std::numeric_limits<uint32_t>::max()) 
            {
                graphics_queue_node_index = i;
            }

            if (supports_present[i] == VK_TRUE) 
            {
                graphics_queue_node_index = i;
                present_queue_node_index = i;
                break;
            }
        }
    }
    if (present_queue_node_index == std::numeric_limits<uint32_t>::max()) 
    {	
        // If there's no queue that supports both present and graphics
        // try to find a separate present queue
        for (uint32_t i = 0; i < queue_count; ++i) 
        {
            if (supports_present[i] == VK_TRUE) 
            {
                present_queue_node_index = i;
                break;
            }
        }
    }

    // Exit if either a graphics or a presenting queue hasn't been found
    if (graphics_queue_node_index == std::numeric_limits<uint32_t>::max() || present_queue_node_index == std::numeric_limits<uint32_t>::max()) 
        throw std::runtime_error("Could not find a graphics and/or presenting queue");

    // todo : Add support for separate graphics and presenting queue
    if (graphics_queue_node_index != present_queue_node_index) 
        throw std::runtime_error("Separate graphics and presenting queues are not supported yet!");

    queue_node_index = graphics_queue_node_index;

    // Get list of supported surface formats
    uint32_t format_count;
    if(fpGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, NULL) != VK_SUCCESS)
        throw std::runtime_error("Can't get physical device surface formats count");

    assert(format_count > 0);

    std::vector<VkSurfaceFormatKHR> surface_formats(format_count);
    if(fpGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, surface_formats.data()) != VK_SUCCESS)
        throw std::runtime_error("Can't get physical device surface formats");

    // If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
    // there is no preferered format, so we assume VK_FORMAT_B8G8R8A8_UNORM
    if ((format_count == 1) && (surface_formats[0].format == VK_FORMAT_UNDEFINED))
    {
        color_format = VK_FORMAT_B8G8R8A8_UNORM;
        color_space  = surface_formats[0].colorSpace;
    }
    else
    {
        // iterate over the list of available surface format and
        // check for the presence of VK_FORMAT_B8G8R8A8_UNORM
        bool found_B8G8R8A8_UNORM = false;
        for (auto&& surfaceFormat : surface_formats)
        {
            if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
            {
                color_format = surfaceFormat.format;
                color_space = surfaceFormat.colorSpace;
                found_B8G8R8A8_UNORM = true;
                break;
            }
        }

        // in case VK_FORMAT_B8G8R8A8_UNORM is not available
        // select the first available color format
        if (!found_B8G8R8A8_UNORM)
        {
            color_format = surface_formats[0].format;
            color_space = surface_formats[0].colorSpace;
        }
    }
}

void Swapchain::connect(VkInstance instance, VkPhysicalDevice physical_device, VkDevice device)
{
    this->instance        = instance;
    this->physical_device = physical_device;
    this->device          = device;
    VK_GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfaceSupportKHR);
    VK_GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfaceCapabilitiesKHR);
    VK_GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfaceFormatsKHR);
    VK_GET_INSTANCE_PROC_ADDR(instance, GetPhysicalDeviceSurfacePresentModesKHR);
    VK_GET_DEVICE_PROC_ADDR(device, CreateSwapchainKHR);
    VK_GET_DEVICE_PROC_ADDR(device, DestroySwapchainKHR);
    VK_GET_DEVICE_PROC_ADDR(device, GetSwapchainImagesKHR);
    VK_GET_DEVICE_PROC_ADDR(device, AcquireNextImageKHR);
    VK_GET_DEVICE_PROC_ADDR(device, QueuePresentKHR);
}

void Swapchain::create(uint32_t *width, uint32_t *height, bool vsync)
{
    VkSwapchainKHR old_swapchain = swapchain;

    // Get physical device surface properties and formats
    VkSurfaceCapabilitiesKHR surface_capabilities;
    if(fpGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &surface_capabilities) != VK_SUCCESS)
        throw std::runtime_error("Can't get physical device surface capabilities count");

    // Get available present modes
    uint32_t present_mode_count;
    if(fpGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, NULL) != VK_SUCCESS)
        throw std::runtime_error("Can't get physical device surface capabilities");

    std::vector<VkPresentModeKHR> present_modes(present_mode_count);
    vk_assert
    (
        fpGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes.data()),
        "Can't get surface present modes"
    );

    VkExtent2D swapchain_extent = {};
    // If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
    if (surface_capabilities.currentExtent.width == (uint32_t)-1)
    {
        // If the surface size is undefined, the size is set to
        // the size of the images requested.
        swapchain_extent.width = *width;
        swapchain_extent.height = *height;
    }
    else
    {
        // If the surface size is defined, the swap chain size must match
        swapchain_extent = surface_capabilities.currentExtent;
        *width = surface_capabilities.currentExtent.width;
        *height = surface_capabilities.currentExtent.height;
    }


    // Select a present mode for the swapchain

    // The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
    // This mode waits for the vertical blank ("v-sync")
    VkPresentModeKHR swapchain_present_mode = VK_PRESENT_MODE_FIFO_KHR;

    // If v-sync is not requested, try to find a mailbox mode
    // It's the lowest latency non-tearing present mode available
    if (!vsync)
    {
        for (size_t i = 0; i < present_mode_count; i++)
        {
            if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                swapchain_present_mode = VK_PRESENT_MODE_MAILBOX_KHR;
                break;
            }
            if ((swapchain_present_mode != VK_PRESENT_MODE_MAILBOX_KHR) && (present_modes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
            {
                swapchain_present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR;
            }
        }
    }

    // Determine the number of images
    uint32_t desired_number_of_swapchain_images = surface_capabilities.minImageCount + 1;
    if ((surface_capabilities.maxImageCount > 0) && (desired_number_of_swapchain_images > surface_capabilities.maxImageCount))
    {
        desired_number_of_swapchain_images = surface_capabilities.maxImageCount;
    }

    // Find the transformation of the surface
    VkSurfaceTransformFlagsKHR pre_transform;
    if (surface_capabilities.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
    {
        // We prefer a non-rotated transform
        pre_transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    }
    else
    {
        pre_transform = surface_capabilities.currentTransform;
    }

    // Find a supported composite alpha format (not all devices support alpha opaque)
    VkCompositeAlphaFlagBitsKHR composite_alpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    // Simply select the first composite alpha format available
    std::vector<VkCompositeAlphaFlagBitsKHR> composite_alpha_flags = {
        VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
    };
    for (auto& composite_alpha_flag : composite_alpha_flags) {
        if (surface_capabilities.supportedCompositeAlpha & composite_alpha_flag) {
            composite_alpha = composite_alpha_flag;
            break;
        };
    }

    VkSwapchainCreateInfoKHR swapchain_create_info = {};
    swapchain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_create_info.pNext = NULL;
    swapchain_create_info.surface = surface;
    swapchain_create_info.minImageCount = desired_number_of_swapchain_images;
    swapchain_create_info.imageFormat = color_format;
    swapchain_create_info.imageColorSpace = color_space;
    swapchain_create_info.imageExtent = { swapchain_extent.width, swapchain_extent.height };
    swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_create_info.preTransform = (VkSurfaceTransformFlagBitsKHR)pre_transform;
    swapchain_create_info.imageArrayLayers = 1;
    swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchain_create_info.queueFamilyIndexCount = 0;
    swapchain_create_info.pQueueFamilyIndices = NULL;
    swapchain_create_info.presentMode = swapchain_present_mode;
    swapchain_create_info.oldSwapchain = old_swapchain;
    // Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
    swapchain_create_info.clipped = VK_TRUE;
    swapchain_create_info.compositeAlpha = composite_alpha;

    // Set additional usage flag for blitting from the swapchain images if supported
    VkFormatProperties format_properties;
    vkGetPhysicalDeviceFormatProperties(physical_device, color_format, &format_properties);
    if ((format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT_KHR) || (format_properties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT)) {
        swapchain_create_info.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }

    vk_assert
    (
        fpCreateSwapchainKHR(device, &swapchain_create_info, nullptr, &swapchain),
        "Can't create swapchain"
    );

    // If an existing swap chain is re-created, destroy the old swap chain
    // This also cleans up all the presentable images
    if (old_swapchain != VK_NULL_HANDLE) 
    { 
        for (uint32_t i = 0; i < image_count; i++)
        {
            vkDestroyImageView(device, buffers[i].view, nullptr);
        }
        fpDestroySwapchainKHR(device, old_swapchain, nullptr);
    }

    vk_assert
    (
        fpGetSwapchainImagesKHR(device, swapchain, &image_count, NULL),
        "Can't enumerate swapchain images"
    );

    // Get the swap chain images
    images.resize(image_count);
    vk_assert
    (
        fpGetSwapchainImagesKHR(device, swapchain, &image_count, images.data()),
        "Can't get swapchain images"
    );

    // Get the swap chain buffers containing the image and imageview
    buffers.resize(image_count);
    for (uint32_t i = 0; i < image_count; i++)
    {
        VkImageViewCreateInfo color_attachment_view = {};
        color_attachment_view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        color_attachment_view.pNext = NULL;
        color_attachment_view.format = color_format;
        color_attachment_view.components = {
            VK_COMPONENT_SWIZZLE_R,
            VK_COMPONENT_SWIZZLE_G,
            VK_COMPONENT_SWIZZLE_B,
            VK_COMPONENT_SWIZZLE_A
        };
        color_attachment_view.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        color_attachment_view.subresourceRange.baseMipLevel = 0;
        color_attachment_view.subresourceRange.levelCount = 1;
        color_attachment_view.subresourceRange.baseArrayLayer = 0;
        color_attachment_view.subresourceRange.layerCount = 1;
        color_attachment_view.viewType = VK_IMAGE_VIEW_TYPE_2D;
        color_attachment_view.flags = 0;

        buffers[i].image = images[i];

        color_attachment_view.image = buffers[i].image;

        vk_assert
        (
            vkCreateImageView(device, &color_attachment_view, nullptr, &buffers[i].view),
            "Can't create swapchain image views"
        );
    }
}

VkResult Swapchain::acquire_next_image(VkSemaphore present_complete_semaphore, uint32_t *image_index)
{
    // By setting timeout to UINT64_MAX we will always wait until the next image has been acquired or an actual error is thrown
    // With that we don't have to handle VK_NOT_READY
    return fpAcquireNextImageKHR(device, swapchain, std::numeric_limits<size_t>::max(), present_complete_semaphore, (VkFence)nullptr, image_index);
}

VkResult Swapchain::queue_present(VkQueue queue, uint32_t image_index, VkSemaphore wait_semaphore)
{
    VkPresentInfoKHR present_create_info = {};
    present_create_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_create_info.pNext = NULL;
    present_create_info.swapchainCount = 1;
    present_create_info.pSwapchains = &swapchain;
    present_create_info.pImageIndices = &image_index;
    // Check if a wait semaphore has been specified to wait for before presenting the image
    if (wait_semaphore != VK_NULL_HANDLE)
    {
        present_create_info.pWaitSemaphores = &wait_semaphore;
        present_create_info.waitSemaphoreCount = 1;
    }
    return fpQueuePresentKHR(queue, &present_create_info);
}

void Swapchain::cleanup()
{
    if (swapchain != VK_NULL_HANDLE)
        for (uint32_t i = 0; i < image_count; i++)
            vkDestroyImageView(device, buffers[i].view, nullptr);

    if (surface != VK_NULL_HANDLE)
    {
        fpDestroySwapchainKHR(device, swapchain, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);
    }
    surface   = VK_NULL_HANDLE;
    swapchain = VK_NULL_HANDLE;
}

const VkFormat &Swapchain::get_color_format() const
{
    return color_format;
}

void Swapchain::set_color_format(const VkFormat &format)
{
    color_format = format;
}

const VkColorSpaceKHR &Swapchain::get_color_space() const
{
    return color_space;
}

void Swapchain::set_color_space(const VkColorSpaceKHR &space)
{
    color_space = space;
}

uint32_t Swapchain::get_image_count() const
{
    return image_count;
}

uint32_t Swapchain::get_queue_node_index() const
{
    return queue_node_index;
}

std::vector<Swapchain::Buffer> &Swapchain::buffers_ref()
{
    return buffers;
}