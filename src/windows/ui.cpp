#include <algorithm>
#include <Windows.h>

#include "../ui.h"
#include "../abstractrenderer.h"

bool Ui::is_stopped = false;
std::vector<AbstractRenderer *> Ui::renderers;

void Ui::execute()
{
    is_stopped = false;

    MSG msg;
    while(!is_stopped)
    {
        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if(msg.message == WM_QUIT)
                stop();
        }

        render_all();
    }
}

void Ui::stop()
{
    is_stopped = true;
}

void Ui::register_renderer(AbstractRenderer &renderer)
{
    if(std::find(renderers.begin(), renderers.end(), &renderer) == renderers.end())
        renderers.push_back(&renderer);
}

void Ui::render_all()
{
    for(auto &&renderer : renderers)
        renderer->render();
}