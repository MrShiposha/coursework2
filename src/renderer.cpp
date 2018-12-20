#include <fstream>
#include <sstream>

#include "renderer.h"
#include "vkassert.h"

VKAPI_ATTR VkBool32 VKAPI_CALL message_callback
(
    VkDebugReportFlagsEXT      flags,
    VkDebugReportObjectTypeEXT obj_type,
    uint64_t                   src_object,
    size_t                     location,
    int32_t                    msg_code,
    const char*                layer_prefix,
    const char*                msg,
    void*                      user_data
);

Renderer::Renderer(std::string_view application_name, Window &window, VulkanValidationMode mode)
: AbstractRenderer(window), application_name(application_name.data()), validation_mode(mode)
{
    initialize_vulkan();
}

Renderer::~Renderer()
{
    if(validation_mode == VulkanValidationMode::ENABLED)
    {
        auto vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
        vkDestroyDebugReportCallbackEXT(instance, debug_report, nullptr);
    }
}

void Renderer::render()
{

}

void Renderer::initialize_vulkan()
{
    create_instance();

    if(validation_mode == VulkanValidationMode::ENABLED)
        setup_debugging(VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT);

    uint32_t gpu_count = 0;
    vk_assert
    (
        vkEnumeratePhysicalDevices(instance, &gpu_count, nullptr),
        "Can't enumerate physical devices"
    );

    assert(gpu_count > 0);

    std::vector<VkPhysicalDevice> physical_devices(gpu_count);
    vk_assert
    (
        vkEnumeratePhysicalDevices(instance, &gpu_count, physical_devices.data()),
        "Can't get physical devices"
    );

    uint32_t selected_device = 0;
    // TODO: select device

    device = std::make_shared<Device>(physical_devices[selected_device]);

    VkPhysicalDeviceFeatures enabled_features = {};
    vk_assert
    (
        device->initialize_logical_device(enabled_features, { VK_KHR_SWAPCHAIN_EXTENSION_NAME }),
        "Can't create logical device"
    );

    vkGetDeviceQueue(*device, device->queue_family_indices.graphics, 0, &queue);

    depth_format = device->get_supported_depth_format();

    swapchain.connect(instance, *device, *device);

    VkSemaphoreCreateInfo semaphore_create_info = {};
    semaphore_create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    
    vk_assert
    (
        vkCreateSemaphore(*device, &semaphore_create_info, nullptr, &semaphores.render_complete),
        "Can't create semaphore for render"
    );

    vk_assert
    (
        vkCreateSemaphore(*device, &semaphore_create_info, nullptr, &semaphores.present_complete),
        "Can't create semaphore for present"
    );

    submit_pipeline_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    submit_info.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.pWaitDstStageMask    = &submit_pipeline_stages;
    submit_info.waitSemaphoreCount   = 1;
    submit_info.pWaitSemaphores      = &semaphores.present_complete;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores    = &semaphores.render_complete;
}

void Renderer::create_instance()
{
    VkApplicationInfo app_info = {};
    app_info.sType             = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName  = application_name.data();
    app_info.pEngineName       = "bmstu coursework";
    app_info.apiVersion        = VK_API_VERSION_1_0;

    std::vector<const char*> instance_extensions = 
    {
        VK_KHR_SURFACE_EXTENSION_NAME,

#if defined (VK_USE_PLATFORM_MACOS_MVK)
        VK_MVK_MACOS_SURFACE_EXTENSION_NAME
#endif // OS
    };

    std::vector<const char*> validation_layers = { "VK_LAYER_LUNARG_standard_validation" };

    if(validation_mode == VulkanValidationMode::ENABLED)
        instance_extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);

    VkInstanceCreateInfo create_info = {};
    create_info.sType                = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo     = &app_info;
    
    if(instance_extensions.size() > 0)
    {
        create_info.enabledExtensionCount   = static_cast<uint32_t>(instance_extensions.size());
        create_info.ppEnabledExtensionNames = instance_extensions.data();
    }

    if(validation_mode == VulkanValidationMode::ENABLED)
    {
        create_info.enabledLayerCount   = static_cast<uint32_t>(validation_layers.size());
        create_info.ppEnabledLayerNames = validation_layers.data();    
    }

    vk_assert
    (
        vkCreateInstance(&create_info, nullptr, &instance),
        "Can't create Vulkan instance"
    );
}

void Renderer::create_command_pool()
{

}

void Renderer::create_command_buffers()
{

}

void Renderer::create_depth_stencil()
{

}

void Renderer::setup_framebuffer()
{

}

void Renderer::setup_renderpass()
{

}

void Renderer::setup_swapchain()
{

}

void Renderer::create_pipeline_cache()
{

}

void Renderer::prepare()
{

}

void Renderer::prepare_frame()
{

}

void Renderer::submit_frame()
{

}

VKAPI_ATTR VkBool32 VKAPI_CALL message_callback
(
    VkDebugReportFlagsEXT      flags,
    VkDebugReportObjectTypeEXT obj_type,
    uint64_t                   src_object,
    size_t                     location,
    int32_t                    msg_code,
    const char*                layer_prefix,
    const char*                msg,
    void*                      user_data
)
{
    static std::ofstream out("debug.log");

    // Select prefix depending on flags passed to the callback
    // Note that multiple flags may be set for a single validation message
    std::string prefix("");

    // Error that may result in undefined behaviour
    if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
    {
        prefix += "ERROR:";
    };
    // Warnings may hint at unexpected / non-spec API usage
    if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
    {
        prefix += "WARNING:";
    };
    // May indicate sub-optimal usage of the API
    if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
    {
        prefix += "PERFORMANCE:";
    };
    // Informal messages that may become handy during debugging
    if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
    {
        prefix += "INFO:";
    }
    // Diagnostic info from the Vulkan loader and layers
    // Usually not helpful in terms of API usage, but may help to debug layer and loader problems 
    if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
    {
        prefix += "DEBUG:";
    }

    // Display message to default output (console/logcat)
    std::stringstream debug_message;
    debug_message << prefix << " [" << layer_prefix << "] Code " << msg_code << " : " << msg;

    out << debug_message.str() << std::endl;

    // The return value of this callback controls wether the Vulkan call that caused
    // the validation message will be aborted or not
    // We return VK_FALSE as we DON'T want Vulkan calls that cause a validation message 
    // (and return a VkResult) to abort
    // If you instead want to have calls abort, pass in VK_TRUE and the function will 
    // return VK_ERROR_VALIDATION_FAILED_EXT 
    return VK_FALSE;
}

void Renderer::setup_debugging(VkDebugReportFlagsEXT flags)
{   
    auto vkCreateDebugReportCallbackEXT  = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
    // auto vkDebugReportMessageEXT         = reinterpret_cast<PFN_vkDebugReportMessageEXT>(vkGetInstanceProcAddr(instance, "vkDebugReportMessageEXT"));

    VkDebugReportCallbackCreateInfoEXT debug_create_info = {};
    debug_create_info.sType       = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
    debug_create_info.pfnCallback = message_callback;
    debug_create_info.flags       = flags;

    vk_assert
    (
        vkCreateDebugReportCallbackEXT(instance, &debug_create_info, nullptr, &debug_report),
        "Can't create debug report"
    );
}

void Renderer::destroy_command_buffers()
{

}
