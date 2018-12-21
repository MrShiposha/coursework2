#include <stdexcept>

#include "initialize.h"
#include "ui.h"
#include "window.h"
#include "messagebox.h"

#include "renderer.h"

void setup_scene(SceneGraph &);

int main() try
{
    initialize();
    Window window;
    Renderer renderer("CG Coursework", window, VulkanValidationMode::ENABLED);


    SceneGraph scenegraph("scenegraph");
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

void setup_scene(SceneGraph &)
{

}