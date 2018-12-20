#ifndef CG_SEM5_SWAPCHAIN_H
#define CG_SEM5_SWAPCHAIN_H

#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>

#include "vkgetprocaddr.h"
#include "window.h"

class Swapchain
{
public:
    struct Buffer
    {
        VkImage     image;
	    VkImageView view;
    };

    Swapchain();

    void initialize_surface(const Window &);
    void connect(VkInstance, VkPhysicalDevice, VkDevice);
    void create(uint32_t *width, uint32_t *height, bool vsync = false);
    VkResult acquire_next_image(VkSemaphore present_complete_semaphore, uint32_t *image_index);
    VkResult queue_present(VkQueue queue, uint32_t image_index, VkSemaphore wait_semaphore = VK_NULL_HANDLE);
    void cleanup();

    const VkFormat &get_color_format() const;
    void set_color_format(const VkFormat &);

    const VkColorSpaceKHR &get_color_space() const;
    void set_color_space(const VkColorSpaceKHR &);

    uint32_t get_image_count() const;

    uint32_t get_queue_node_index() const;

    std::vector<Buffer> &buffers_ref();

private:
    VkInstance       instance;
    VkPhysicalDevice physical_device;
    VkDevice         device;
    VkSurfaceKHR     surface;

    // Function pointers
	PFN_vkGetPhysicalDeviceSurfaceSupportKHR      fpGetPhysicalDeviceSurfaceSupportKHR;
	PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR fpGetPhysicalDeviceSurfaceCapabilitiesKHR; 
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR      fpGetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkGetPhysicalDeviceSurfacePresentModesKHR fpGetPhysicalDeviceSurfacePresentModesKHR;
	PFN_vkCreateSwapchainKHR                      fpCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR                     fpDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR                   fpGetSwapchainImagesKHR;
	PFN_vkAcquireNextImageKHR                     fpAcquireNextImageKHR;
	PFN_vkQueuePresentKHR                         fpQueuePresentKHR;

    VkFormat        color_format;
    VkColorSpaceKHR color_space;

    VkSwapchainKHR       swapchain;
    uint32_t             image_count;
    std::vector<VkImage> images;
    std::vector<Buffer>  buffers;
    uint32_t             queue_node_index;
};

#endif // CG_SEM5_SWAPCHAIN_H