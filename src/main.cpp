#include <stdexcept>

#include "initialize.h"
#include "ui.h"
#include "window.h"
#include "messagebox.h"

#include "renderer.h"
#include "camera.h"

void setup_scene(SceneGraph &);

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

    scenegraph.add_node(camera);

    setup_scene(scenegraph);

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

void setup_scene(SceneGraph &scene)
{

}