#include <fstream>
#include <sstream>
#include <chrono>

#include "renderer.h"
#include "vkassert.h"
#include "staticmesh.h"

#include "detail/materialcountervisitor.h"

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
: AbstractRenderer(window), application_name(application_name.data()), validation_mode(mode),
width(window.get_view_size().width), height(window.get_view_size().height),
is_prepared(false),
is_view_updated(false),
timer(0.0),
timer_speed(0.25),
frame_counter(0),
frame_timer(0.0), fps_timer(0.0), last_fps(0.0),
current_buffer(0)
{
    initialize_vulkan();
    initialize();
}

Renderer::~Renderer()
{
    free_debugging();

    swapchain.cleanup();

    if(descriptor_pool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(*device, descriptor_pool, nullptr);

    destroy_command_buffers();
    vkDestroyRenderPass(*device, renderpass, nullptr);

    for(uint32_t i = 0; i < framebuffers.size(); ++i)
        vkDestroyFramebuffer(*device, framebuffers[i], nullptr);

    // TODO: Destroy shader modules
    for(auto &&shader_stage : shader_stages)
        if(shader_stage.module != VK_NULL_HANDLE)
            vkDestroyShaderModule(*device, shader_stage.module, nullptr),   

    vkDestroyImageView(*device, depth_stencil.view, nullptr);
	vkDestroyImage(*device, depth_stencil.image, nullptr);
	vkFreeMemory(*device, depth_stencil.memory, nullptr);

	vkDestroyPipelineCache(*device, pipeline_cache, nullptr);

	vkDestroyCommandPool(*device, command_pool, nullptr);

	vkDestroySemaphore(*device, semaphores.present_complete, nullptr);
	vkDestroySemaphore(*device, semaphores.render_complete, nullptr);

    device.reset();

    vkDestroyInstance(instance, nullptr);
}

void Renderer::render()
{
    using namespace std::chrono;

    auto time_start = high_resolution_clock::now();
    if(is_view_updated)
    {
        is_view_updated = false;
        view_changed();
    }

    draw();
    ++frame_counter;

    auto time_end = high_resolution_clock::now();
    auto time_diff = duration_cast<milliseconds>(time_end - time_start).count();
    frame_timer = static_cast<double>(time_diff) / 1000.0;

    // TODO: Update active camera

    timer += timer_speed * frame_timer;
    if(timer > 1.0)
        timer -= 1.0;

    fps_timer += time_diff;
    if(fps_timer > 1000.0)
    {
        last_fps = static_cast<uint32_t>(static_cast<double>(frame_counter) * (1000.0 / fps_timer));
        window.set_title(application_name + " fps: " + std::to_string(last_fps));

        fps_timer     = 0.0;
        frame_counter = 0;
    }

    // vkDeviceWaitIdle(*device);
}

void Renderer::draw()
{
    if(is_prepared)
    {
        prepare_frame();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers    = &draw_command_buffers[current_buffer];

        vk_assert
        (
            vkQueueSubmit(queue, 1, &submit_info, VK_NULL_HANDLE),
            "Can't submit frame"
        );

        submit_frame();
    }
}

void Renderer::on_window_resize()
{
    if(is_left_mouse_button_pressed())
    {
        // TODO: Camera transforms
        is_view_updated = true;
    }
}

void Renderer::initialize()
{
    initialize_swapchain();
    create_command_pool();
    setup_swapchain();
    create_command_buffers();
    create_depth_stencil();
    setup_renderpass();
    create_pipeline_cache();
    setup_framebuffer();
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
    command_pool = device->create_command_pool(device->queue_family_indices.graphics, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
}

void Renderer::create_command_buffers()
{ 
    draw_command_buffers.resize(swapchain.get_image_count());
    VkCommandBufferAllocateInfo allocate_info = {};
    allocate_info.sType                       = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.commandPool                 = command_pool;
    allocate_info.level                       = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount          = static_cast<uint32_t>(draw_command_buffers.size());

    vk_assert
    (
        vkAllocateCommandBuffers(*device, &allocate_info, draw_command_buffers.data()),
        "Can't create draw command buffers"
    );
}

void Renderer::create_depth_stencil()
{
    VkImageCreateInfo image_create_info = {};
    image_create_info.sType             = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_create_info.imageType         = VK_IMAGE_TYPE_2D;
    image_create_info.format            = depth_format;
    image_create_info.extent            = { width, height, 1 };
    image_create_info.mipLevels         = 1;
    image_create_info.arrayLayers       = 1;
    image_create_info.samples           = VK_SAMPLE_COUNT_1_BIT;
    image_create_info.tiling            = VK_IMAGE_TILING_OPTIMAL;
    image_create_info.usage             = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    image_create_info.flags             = 0;

    VkMemoryAllocateInfo mem_alloc = {};
	mem_alloc.sType                = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc.allocationSize       = 0;
	mem_alloc.memoryTypeIndex      = 0;

	VkImageViewCreateInfo depth_stencil_view           = {};
	depth_stencil_view.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	depth_stencil_view.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
	depth_stencil_view.format                          = depth_format;
	depth_stencil_view.flags                           = 0;
	depth_stencil_view.subresourceRange                = {};
	depth_stencil_view.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_DEPTH_BIT 
                                                       | VK_IMAGE_ASPECT_STENCIL_BIT;
	depth_stencil_view.subresourceRange.baseMipLevel   = 0;
	depth_stencil_view.subresourceRange.levelCount     = 1;
	depth_stencil_view.subresourceRange.baseArrayLayer = 0;
	depth_stencil_view.subresourceRange.layerCount     = 1;

	VkMemoryRequirements mem_reqs;

    vk_assert
    (
        vkCreateImage(*device, &image_create_info, nullptr, &depth_stencil.image),
        "Can't create depth stencil image"
    );

    vkGetImageMemoryRequirements(*device, depth_stencil.image, &mem_reqs);
	mem_alloc.allocationSize = mem_reqs.size;
	mem_alloc.memoryTypeIndex = device->find_memory_type(mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT).value();
	vk_assert
    (
        vkAllocateMemory(*device, &mem_alloc, nullptr, &depth_stencil.memory),
        "Can't allocate memory for depth stencil"
    );

	vk_assert
    (
        vkBindImageMemory(*device, depth_stencil.image, depth_stencil.memory, 0),
        "Can't bind depth stencil image memory"
    );

	depth_stencil_view.image = depth_stencil.image;
	vk_assert
    (
        vkCreateImageView(*device, &depth_stencil_view, nullptr, &depth_stencil.view),
        "Can't create depth stencil image view"
    );
}

void Renderer::setup_framebuffer()
{
    VkImageView attachments[2];

    // Depth/Stencil attachment is the same for all frame buffers
    attachments[1] = depth_stencil.view;

    VkFramebufferCreateInfo framebuffer_create_info = {};
    framebuffer_create_info.sType                   = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_create_info.renderPass              = renderpass;
    framebuffer_create_info.attachmentCount         = 2;
    framebuffer_create_info.pAttachments            = attachments;
    framebuffer_create_info.width                   = width;
    framebuffer_create_info.height                  = height;
    framebuffer_create_info.layers                  = 1;

    // Create frame buffers for every swap chain image
    framebuffers.resize(swapchain.get_image_count());
    for(uint32_t i = 0; i < framebuffers.size(); ++i)
    {
        attachments[0] = swapchain.buffers_ref()[i].view;
        vk_assert
        (
            vkCreateFramebuffer(*device, &framebuffer_create_info, nullptr, &framebuffers[i]),
            "Can't create framebuffers"
        );
    }
}

void Renderer::setup_renderpass()
{
    std::array<VkAttachmentDescription, 2> attachments = {};
    // Color attachment
	attachments[0].format         = swapchain.get_color_format();
	attachments[0].samples        = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[0].finalLayout    = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	// Depth attachment
	attachments[1].format         = depth_format;
	attachments[1].samples        = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp         = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp        = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED;
	attachments[1].finalLayout    = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference color_reference = {};
    color_reference.attachment            = 0;
    color_reference.layout                = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_reference = {};
    depth_reference.attachment            = 1;
    depth_reference.layout                = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass_description    = {};
	subpass_description.pipelineBindPoint       = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass_description.colorAttachmentCount    = 1;
	subpass_description.pColorAttachments       = &color_reference;
	subpass_description.pDepthStencilAttachment = &depth_reference;
	subpass_description.inputAttachmentCount    = 0;
	subpass_description.pInputAttachments       = nullptr;
	subpass_description.preserveAttachmentCount = 0;
	subpass_description.pPreserveAttachments    = nullptr;
	subpass_description.pResolveAttachments     = nullptr;

    // Subpass dependencies for layout transitions
	std::array<VkSubpassDependency, 2> dependencies;
    dependencies[0].srcSubpass      = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass      = 0;
	dependencies[0].srcStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass      = 0;
	dependencies[1].dstSubpass      = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask    = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask    = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask   = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderpass_create_info = {};
    renderpass_create_info.sType                  = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpass_create_info.attachmentCount        = static_cast<uint32_t>(attachments.size());
	renderpass_create_info.pAttachments           = attachments.data();
	renderpass_create_info.subpassCount           = 1;
	renderpass_create_info.pSubpasses             = &subpass_description;
	renderpass_create_info.dependencyCount        = static_cast<uint32_t>(dependencies.size());
	renderpass_create_info.pDependencies          = dependencies.data();

    vk_assert
    (
        vkCreateRenderPass(*device, &renderpass_create_info, nullptr, &renderpass),
        "Can't create render pass"
    );
}

void Renderer::setup_swapchain()
{
    swapchain.create(&width, &height);
}

void Renderer::initialize_swapchain()
{
#if defined(VK_USE_PLATFORM_MACOS_MVK)
    swapchain.initialize_surface(this->window);
#endif // OS
}

void Renderer::create_pipeline_cache()
{
    VkPipelineCacheCreateInfo pipeline_cache_create_info = {};
    pipeline_cache_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    vk_assert
    (
        vkCreatePipelineCache(*device, &pipeline_cache_create_info, nullptr, &pipeline_cache),
        "Can't create pipeline cache"
    );
}

void Renderer::prepare(SceneGraph &scenegraph)
{
    create_static_mesh_vertex_descriptions();

    {
        MaterialCounterVisitor counter;
        scenegraph.accept_down(counter);

        setup_descriptor_pool(counter.get_materials_count());
    }

    {
        setup_scene_descriptor_set_layout();
        setup_materials_descriptor_set_layout();
        setup_static_mesh_pipeline_layout();
    }

    create_pipelines();
    fill_command_buffers();

}

void Renderer::prepare_frame()
{
    VkResult err = swapchain.acquire_next_image(semaphores.present_complete, &current_buffer);

    // TODO: Resize
    vk_assert(err, "Can't prepare frame");
}

void Renderer::submit_frame()
{
    vk_assert
    (
        swapchain.queue_present(queue, current_buffer, semaphores.render_complete),
        "Can't present frame"
    );

    vk_assert
    (
        vkQueueWaitIdle(queue),
        "Can't wait queue"
    );
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

void Renderer::free_debugging()
{
    if(validation_mode == VulkanValidationMode::ENABLED)
    {
        auto vkDestroyDebugReportCallbackEXT = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
        vkDestroyDebugReportCallbackEXT(instance, debug_report, nullptr);
    }
}

void Renderer::create_static_mesh_vertex_descriptions()
{

    vertex_info.static_mesh.binding_descriptions.resize(1);

    VkVertexInputBindingDescription input_binding_description = {};
    input_binding_description.binding   = STATIC_MESH_BUFFER_ID;
    input_binding_description.stride    = sizeof(StaticMesh::Vertex);
    input_binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    vertex_info.static_mesh.binding_descriptions[0] = input_binding_description;

    VkVertexInputAttributeDescription attribute_description = {};
    vertex_info.static_mesh.attribute_descriptions.resize(4);

    // Position (loc = 0)
    attribute_description.location = 0;
    attribute_description.binding  = STATIC_MESH_BUFFER_ID;
    attribute_description.format   = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_description.offset   = 0;
    vertex_info.static_mesh.attribute_descriptions[0] = attribute_description;

    // Normal (loc = 1)
    attribute_description.location = 1;
    attribute_description.binding  = STATIC_MESH_BUFFER_ID;
    attribute_description.format   = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_description.offset   = sizeof(float) * 3;
    vertex_info.static_mesh.attribute_descriptions[1] = attribute_description;

    // UV (loc = 2)
    attribute_description.location = 2;
    attribute_description.binding  = STATIC_MESH_BUFFER_ID;
    attribute_description.format   = VK_FORMAT_R32G32_SFLOAT;
    attribute_description.offset   = sizeof(float) * 6;
    vertex_info.static_mesh.attribute_descriptions[2] = attribute_description;

    // Color (loc = 3)
    attribute_description.location = 3;
    attribute_description.binding  = STATIC_MESH_BUFFER_ID;
    attribute_description.format   = VK_FORMAT_R32G32B32_SFLOAT;
    attribute_description.offset   = sizeof(float) * 8;
    vertex_info.static_mesh.attribute_descriptions[3] = attribute_description;

    vertex_info.static_mesh.input_state = {};
    vertex_info.static_mesh.input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_info.static_mesh.input_state.vertexBindingDescriptionCount   = static_cast<uint32_t>(vertex_info.static_mesh.binding_descriptions.size());
    vertex_info.static_mesh.input_state.pVertexBindingDescriptions      = vertex_info.static_mesh.binding_descriptions.data();
    vertex_info.static_mesh.input_state.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_info.static_mesh.attribute_descriptions.size());
    vertex_info.static_mesh.input_state.pVertexAttributeDescriptions    = vertex_info.static_mesh.attribute_descriptions.data();
}

void Renderer::create_pipelines()
{
    // Static mesh
    VkPipelineInputAssemblyStateCreateInfo assembly_create_info = {};
    assembly_create_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    assembly_create_info.topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    assembly_create_info.flags                  = 0;
    assembly_create_info.primitiveRestartEnable = VK_FALSE;

    VkPipelineRasterizationStateCreateInfo rasterization_create_info = {};
    rasterization_create_info.sType            = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_create_info.polygonMode      = VK_POLYGON_MODE_FILL;
    rasterization_create_info.cullMode         = VK_CULL_MODE_NONE; // !
    rasterization_create_info.frontFace        = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterization_create_info.flags            = 0;
    rasterization_create_info.depthClampEnable = VK_FALSE;
    rasterization_create_info.lineWidth        = 1.f;

    VkPipelineColorBlendAttachmentState blend_attachment_state = {};
    blend_attachment_state.colorWriteMask = 0xF;
    blend_attachment_state.blendEnable    = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {};
    color_blend_state_create_info.sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blend_state_create_info.attachmentCount = 1;
    color_blend_state_create_info.pAttachments    = &blend_attachment_state;

    VkPipelineDepthStencilStateCreateInfo depth_stencil_create_info = {};
    depth_stencil_create_info.sType            = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil_create_info.depthTestEnable  = VK_TRUE;
    depth_stencil_create_info.depthWriteEnable = VK_TRUE;
    depth_stencil_create_info.depthCompareOp   = VK_COMPARE_OP_LESS_OR_EQUAL;
    depth_stencil_create_info.front            = depth_stencil_create_info.back;
    depth_stencil_create_info.back.compareOp   = VK_COMPARE_OP_ALWAYS;

    VkPipelineViewportStateCreateInfo viewport_state_create_info = {};
    viewport_state_create_info.sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state_create_info.viewportCount = 1;
    viewport_state_create_info.scissorCount  = 1;
    viewport_state_create_info.flags         = 0;

    VkPipelineMultisampleStateCreateInfo multisample_create_info = {};
    multisample_create_info.sType                = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample_create_info.flags                = 0;

    std::vector<VkDynamicState> dynamic_state_enables = 
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {};
    dynamic_state_create_info.sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state_create_info.dynamicStateCount = static_cast<uint32_t>(dynamic_state_enables.size());
    dynamic_state_create_info.pDynamicStates    = dynamic_state_enables.data();
    dynamic_state_create_info.flags             = 0;

    shader_stages[0] = device->load_shader("resources/shaders/static_mesh.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    shader_stages[1] = device->load_shader("resources/shaders/static_mesh.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    VkGraphicsPipelineCreateInfo pipeline_create_info = {};
    pipeline_create_info.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_create_info.layout              = pipeline_layouts.static_mesh;
    pipeline_create_info.renderPass          = renderpass;
    pipeline_create_info.flags               = 0;
    pipeline_create_info.basePipelineIndex   = -1;
    pipeline_create_info.basePipelineHandle  = VK_NULL_HANDLE;
    pipeline_create_info.pVertexInputState   = &vertex_info.static_mesh.input_state;
    pipeline_create_info.pInputAssemblyState = &assembly_create_info;
    pipeline_create_info.pRasterizationState = &rasterization_create_info;
    pipeline_create_info.pColorBlendState    = &color_blend_state_create_info;
    pipeline_create_info.pMultisampleState   = &multisample_create_info;
    pipeline_create_info.pViewportState      = &viewport_state_create_info;
    pipeline_create_info.pDepthStencilState  = &depth_stencil_create_info;
    pipeline_create_info.pDynamicState       = &dynamic_state_create_info;
    pipeline_create_info.stageCount          = static_cast<uint32_t>(shader_stages.size());
    pipeline_create_info.pStages             = shader_stages.data();

    // Enabled Alpha blending
    blend_attachment_state.blendEnable         = VK_TRUE;
    blend_attachment_state.colorBlendOp        = VK_BLEND_OP_ADD;
    blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
    blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA; // ! _COLOR

    vk_assert
    (
        vkCreateGraphicsPipelines(*device, pipeline_cache, 1, &pipeline_create_info, nullptr, &pipelines.static_mesh),
        "Can't create graphics pipeline for static meshes"
    );
}

void Renderer::fill_command_buffers()
{
    VkCommandBufferBeginInfo buffer_begin_info = {};
    buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VkClearValue clear_values[2];
    clear_values[0].color        = clear_color;
    clear_values[1].depthStencil = { 1.f, 0 };

    VkRenderPassBeginInfo renderpass_begin_info    = {};
    renderpass_begin_info.sType                    = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderpass_begin_info.renderPass               = renderpass;
    renderpass_begin_info.renderArea.offset.x      = 0;
    renderpass_begin_info.renderArea.offset.y      = 0;
    renderpass_begin_info.renderArea.extent.width  = width;
    renderpass_begin_info.renderArea.extent.height = height;
    renderpass_begin_info.clearValueCount          = 2;
    renderpass_begin_info.pClearValues             = clear_values;

    for(uint32_t i = 0; i < draw_command_buffers.size(); ++i)
    {
        renderpass_begin_info.framebuffer = framebuffers[i];

        vk_assert
        (
            vkBeginCommandBuffer(draw_command_buffers[i], &buffer_begin_info),
            "Can't begin draw buffer"
        );

        vkCmdBeginRenderPass(draw_command_buffers[i], &renderpass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport = {};
        viewport.width      = width;
        viewport.height     = height;
        viewport.minDepth   = 0.f;
        viewport.maxDepth   = 1.f;
        vkCmdSetViewport(draw_command_buffers[i], 0, 1, &viewport);

        VkRect2D scissor = {};
        scissor.extent.width    = width;
        scissor.extent.height   = height;
        scissor.offset.x        = 0;
        scissor.offset.y        = 0;
        vkCmdSetScissor(draw_command_buffers[i], 0, 1, &scissor);

        // TODO: Actual rendering

        vkCmdEndRenderPass(draw_command_buffers[i]);

        vk_assert
        (
            vkEndCommandBuffer(draw_command_buffers[i]),
            "Can't finish draw command buffer"
        );
    }
}

void Renderer::setup_static_mesh_pipeline_layout()
{
    std::array<VkDescriptorSetLayout, 2> layouts = 
    {
        descriptor_set_layouts.scene,
        descriptor_set_layouts.static_mesh_material
    };

    // Use push constants to pass material properties to the fragment shader
    VkPushConstantRange push_constant_range = {};
    push_constant_range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    push_constant_range.size       = sizeof(StaticMesh::MaterialProperties);
    push_constant_range.offset     = 0;

    VkPipelineLayoutCreateInfo layout_create_info = {};
    layout_create_info.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    layout_create_info.setLayoutCount         = static_cast<uint32_t>(layouts.size());
    layout_create_info.pSetLayouts            = layouts.data();
    layout_create_info.pushConstantRangeCount = 1;
    layout_create_info.pPushConstantRanges    = &push_constant_range;

    vk_assert
    (
        vkCreatePipelineLayout(*device, &layout_create_info, nullptr, &pipeline_layouts.static_mesh),
        "Can't create pipeline layout for static mesh"
    );
}

void Renderer::setup_static_mesh_descriptors()
{

}

void Renderer::setup_descriptor_pool(uint32_t samplers_count)
{
    std::vector<VkDescriptorPoolSize> pool_sizes = 
    {
        VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         1 },
        VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1 }
    };

    if(samplers_count > 0)
        pool_sizes.push_back(VkDescriptorPoolSize { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, samplers_count });

    VkDescriptorPoolCreateInfo pool_create_info = {};
    pool_create_info.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_create_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.size());
    pool_create_info.pPoolSizes    = pool_sizes.data();
    pool_create_info.maxSets       = samplers_count + 1; // + 1 for uniforms (static and dynamic on one DescriptorSet)

    vk_assert
    (
        vkCreateDescriptorPool(*device, &pool_create_info, nullptr, &descriptor_pool),
        "Can't create descriptor pool"
    );
}

void Renderer::setup_scene_descriptor_set_layout()
{
    std::vector<VkDescriptorSetLayoutBinding> layout_bindings = 
    {
        VkDescriptorSetLayoutBinding { 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr },
        VkDescriptorSetLayoutBinding { 1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr }
    };

    VkDescriptorSetLayoutCreateInfo descriptor_layout_create_info = {};
    descriptor_layout_create_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_layout_create_info.bindingCount = static_cast<uint32_t>(layout_bindings.size());
    descriptor_layout_create_info.pBindings    = layout_bindings.data();

    vk_assert
    (
        vkCreateDescriptorSetLayout(*device, &descriptor_layout_create_info, nullptr, &descriptor_set_layouts.scene),
        "Can't create descriptor set layout for scene"
    );
}

void Renderer::setup_materials_descriptor_set_layout()
{
    std::vector<VkDescriptorSetLayoutBinding> layout_bindings = 
    {
        VkDescriptorSetLayoutBinding { 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr }
    };

    VkDescriptorSetLayoutCreateInfo descriptor_layout_create_info = {};
    descriptor_layout_create_info.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_layout_create_info.bindingCount = static_cast<uint32_t>(layout_bindings.size());
    descriptor_layout_create_info.pBindings    = layout_bindings.data();

    vk_assert
    (
        vkCreateDescriptorSetLayout(*device, &descriptor_layout_create_info, nullptr, &descriptor_set_layouts.static_mesh_material),
        "Can't create descriptor set layout for static mesh materials"
    );
}

void Renderer::view_changed()
{
    // TODO:
}

void Renderer::destroy_command_buffers()
{
    vkFreeCommandBuffers(*device, command_pool, static_cast<uint32_t>(draw_command_buffers.size()), draw_command_buffers.data());
}

std::shared_ptr<Device> Renderer::get_device() const
{
    return device;
}

VkCommandPool Renderer::get_command_pool() const
{
    return command_pool;
}

VkQueue Renderer::get_queue() const
{
    return queue;
}
