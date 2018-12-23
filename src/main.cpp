#include <stdexcept>

#include "initialize.h"
#include "ui.h"
#include "window.h"
#include "messagebox.h"

#include "renderer.h"
#include "camera.h"
#include "staticmesh.h"
#include "flame.h"

void setup_scene(Renderer &renderer, SceneGraph &);

int main() try
{
    initialize();
    Window window;
    Renderer renderer("CG Coursework", window, VulkanValidationMode::ENABLED);


    SceneGraph scenegraph("scenegraph");
    auto view_size = window.get_view_size();
    float aspect_ratio = static_cast<float>(view_size.width) / static_cast<float>(view_size.height);

    auto camera = std::make_shared<Camera>
    (
        60.f,
        aspect_ratio,
        0.1f,
        256.f
    );

    // camera->translate(glm::vec3(0.0, -0.5f, -1.15f));

    scenegraph.add_node(camera);

    setup_scene(renderer, scenegraph);

    renderer.prepare(scenegraph);

    Ui::register_renderer(renderer);

    window.show();

    Ui::execute();
    return 0;
}
catch(const std::exception &e)
{
    message_box("Error", e.what());
    Ui::stop();
    return -1;
}

void setup_scene(Renderer &renderer, SceneGraph &scene)
{
    auto mesh = StaticMesh::load_from_file
    (
        "poleno", 
        "resources/obj/poleno/poleno.obj", 
        renderer.get_device(),
        renderer.get_command_pool(),
        renderer.get_queue()
    );

    mesh->scale(glm::vec3(0.5f, 0.5f, 0.5f));
    mesh->translate(glm::vec3(0.f, -1.f, 0.f));

    scene.add_node(mesh);

    auto flame_texture = Texture2D::load_from_file
    (
        "resources/textures/flame_particle.ktx",
        VK_FORMAT_R8G8B8A8_UNORM,
        renderer.get_device(),
        renderer.get_command_pool(),
        renderer.get_queue()
    );

    auto flame = std::make_shared<Flame>
    (
        "center flame",
        128,  // particle count
        4.4f,  // radius
        3.5f, // alpha damping
        1.f,  // size damping
        2.f,  // alpha threshold
        glm::vec3(0.f, 3.5f, 0.f),   // emitter position
        glm::vec3(-3.f, 0.5f, -3.f), // min velocity
        glm::vec3(3.f, 4.5f, 3.f),   // max velocity
        flame_texture
    );

    flame->scale(glm::vec3(0.2f, 0.2f, 0.2f));

    scene.add_node(flame);


    flame = std::make_shared<Flame>
    (
        "left flame",
        256, // particle count
        2.5f, // radius
        2.f, // alpha damping
        1.f, // size damping
        2.f, // alpha threshold
        glm::vec3(-13.0f, -1.f, 3.f),   // emitter position
        glm::vec3(-3.f, 0.5f, -3.f), // min velocity
        glm::vec3(3.f, 4.5f, 3.f),   // max velocity
        flame_texture
    );

    flame->scale(glm::vec3(0.2f, 0.2f, 0.2f));

    scene.add_node(flame);


    flame = std::make_shared<Flame>
    (
        "right flame",
        256, // particle count
        1.f, // radius
        2.f, // alpha damping
        2.f, // size damping
        2.f, // alpha threshold
        glm::vec3(8.3f, 1.f, -1.f),   // emitter position
        glm::vec3(-3.f, 1.5f, -3.f), // min velocity
        glm::vec3(3.f, 7.5f, 3.f),   // max velocity
        flame_texture
    );

    flame->scale(glm::vec3(0.2f, 0.2f, 0.2f));

    scene.add_node(flame);
}