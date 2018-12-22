#include <stdexcept>

#include "initialize.h"
#include "ui.h"
#include "window.h"
#include "messagebox.h"

#include "renderer.h"
#include "camera.h"
#include "staticmesh.h"

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

    camera->translate(glm::vec3(0.0, -0.5f, -1.15f));

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
        "cat", 
        "resources/obj/cat/cat.obj", 
        renderer.get_device(),
        renderer.get_command_pool(),
        renderer.get_queue()
    );

    scene.add_node(mesh);
}