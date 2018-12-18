#import <Foundation/Foundation.h>
#import <AppKit/AppKit.h>

#include <algorithm>

#include "../ui.h"
#include "../abstractrenderer.h"

bool Ui::is_stopped = false;
std::vector<AbstractRenderer *> Ui::renderers;

void Ui::execute()
{
    is_stopped = false;

    NSEvent *event = nullptr;
    while(!is_stopped)
    {
        event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:nil inMode:NSDefaultRunLoopMode dequeue:YES];
        switch ([event type])
        {
        default:
            [NSApp sendEvent:event];
            break;
        }
        
        [event release];
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