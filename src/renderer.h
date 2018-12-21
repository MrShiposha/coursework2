#ifndef CG_SEM5_RENDERER_H
#define CG_SEM5_RENDERER_H

#include <memory>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

#include "abstractrenderer.h"
#include "device.h"
#include "devicebuffer.h"
#include "swapchain.h"

enum class VulkanValidationMode
{
    DISABLED,
    ENABLED
};

class Renderer : public AbstractRenderer
{
public:
    Renderer
    (
        std::string_view application_name,
        Window &,
        VulkanValidationMode
    );

    ~Renderer();

    virtual void render() override;
    virtual void on_window_resize() override;

    void initialize_vulkan();
    void create_instance();
    void create_command_pool();
    void create_command_buffers();
    void create_depth_stencil();
    void setup_framebuffer();
    void setup_renderpass();
    void setup_swapchain();
    void initialize_swapchain();
    void create_pipeline_cache();
    void prepare();
    void prepare_frame();
    void submit_frame();

    void setup_debugging(VkDebugReportFlagsEXT flags);
    void free_debugging();

    void view_changed();

    void destroy_command_buffers();

private:
    void draw();

    ////////////////////////////////////////////
    //           Vulkan must have             //
    ////////////////////////////////////////////

    std::string          application_name;
    VulkanValidationMode validation_mode;

    VkDebugReportCallbackEXT debug_report;

    VkInstance instance;
    std::shared_ptr<Device> device;
    VkQueue queue;
    
    VkFormat depth_format;
    struct
    {
        VkImage        image;
        VkDeviceMemory memory;
        VkImageView    view;
    } depth_stencil;

    uint32_t width, height;

    struct
    {
        VkSemaphore render_complete;
        VkSemaphore present_complete;
    } semaphores;

    VkPipelineStageFlags submit_pipeline_stages;
    VkSubmitInfo submit_info;

    Swapchain swapchain;
    std::vector<VkFramebuffer> framebuffers;

    VkRenderPass renderpass;

    VkPipelineCache pipeline_cache;

    VkCommandPool command_pool;
    std::vector<VkCommandBuffer> draw_command_buffers;
    uint32_t current_buffer;

    bool is_prepared;

    ////////////////////////////////////////////
    //           Project specific             //
    ////////////////////////////////////////////

    bool is_view_updated;

    double timer;
    double timer_speed;

    uint32_t frame_counter;
    double   frame_timer;
    double   fps_timer;
    double   last_fps;

    VkDescriptorPool descriptor_pool;

    struct 
    {
        VkDescriptorSetLayout static_mesh_material;
        VkDescriptorSetLayout scene;
    } descriptor_set_layouts;

    VkDescriptorSet scene_descriptor_set;

    std::shared_ptr<DeviceBuffer> vertex_buffer;
    std::shared_ptr<DeviceBuffer> index_buffer;

    struct 
    {
        std::shared_ptr<DeviceBuffer> static_uniform;
        std::shared_ptr<DeviceBuffer> dynamic_uniform;
    } uniform_buffers;

    struct StaticUniformData
    {
        glm::mat4 projection;
        glm::mat4 view;
        glm::vec4 light_position = glm::vec4(1.25f, 8.35f, 0.0f, 0.0f);
    } static_uniform_data;

    struct DynamicUniformData
    {
        glm::mat4 *models = nullptr;
    } dynamic_uniform_data;

    struct 
    {
        struct 
        {
            VkPipelineVertexInputStateCreateInfo           input_state;
            std::vector<VkVertexInputBindingDescription>   binding_descriptions;
            std::vector<VkVertexInputAttributeDescription> attribute_descriptions;
        } static_mesh;
    } vertex_info;

    struct 
    {
        VkPipeline static_mesh;
    } pipelines;

    VkPipelineLayout pipeline_layout;

};

#endif // CG_SEM5_RENDERER_H