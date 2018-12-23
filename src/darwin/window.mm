#import <Foundation/Foundation.h>
#import <Cocoa/Cocoa.h>
#import <QuartzCore/CAMetalLayer.h>

#include "../window.h"
#import "vulkanview.h"
#import "windowdelegate.h"

constexpr float DEFAULT_WIDTH  = 1024;
constexpr float DEFAULT_HEIGHT = 768;

Window::Window()
: handle
(
    [[WindowDelegate alloc]
        initWithContentRect:NSMakeRect(0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT)
        styleMask: NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable /*| NSWindowStyleMaskResizable */
        backing:NSBackingStoreBuffered
        defer:NO
    ]
)
{
    WindowDelegate *window = static_cast<id>(handle);
    [window init_default_callbacks];

    VulkanView *view = [[VulkanView alloc] initWithFrame: NSMakeRect(0, 0, DEFAULT_WIDTH, DEFAULT_HEIGHT)];
    [view setWantsLayer:YES];
    [window setContentView:view];  
    [window setAcceptsMouseMovedEvents:YES];
}

Window::Window(const std::string &title)
: Window()
{
    set_title(title);
}

Window::~Window()
{
    id handle = static_cast<id>(this->handle);
    [[handle contentView] release];
    [handle release];
}

void *Window::get_view() const
{
    return [static_cast<id>(handle) contentView];
}

void Window::set_title(const std::string &title)
{
    [static_cast<id>(handle) setTitle:[NSString stringWithUTF8String:title.c_str()]];
}

std::string Window::get_title() const
{
    return [static_cast<WindowDelegate*>(handle).title UTF8String];
}

void Window::set_size(const Size &size)
{
    NSRect frame = static_cast<WindowDelegate*>(handle).frame;
    [static_cast<id>(handle) setFrame:NSMakeRect(frame.origin.x, frame.origin.y, size.width, size.height)
                             display:YES
                             animate:YES];
}

Window::Size Window::get_size() const
{
    NSRect frame = static_cast<WindowDelegate*>(handle).frame;
    return Size {static_cast<uint32_t>(frame.size.width), static_cast<uint32_t>(frame.size.height)};
}

void Window::set_view_size(const Size &size)
{
    NSRect frame = [static_cast<id>(handle) contentView].frame;
    NSRect new_view_frame = NSMakeRect(frame.origin.x, frame.origin.y, size.width, size.height);

    [static_cast<id>(handle) setContentSize:new_view_frame.size];
    [static_cast<id>(handle) frameRectForContentRect:new_view_frame];
}

Window::Size Window::get_view_size() const
{
    NSRect frame = [static_cast<WindowDelegate*>(handle) contentView].frame;
    return Size {static_cast<uint32_t>(frame.size.width), static_cast<uint32_t>(frame.size.height)};
}

void Window::show()
{
    [static_cast<id>(handle) makeKeyAndOrderFront:nil];
    [NSApp activateIgnoringOtherApps:YES];
}

void Window::hide()
{
    [static_cast<id>(handle) orderOut:nil];
}

void Window::set_resize_callback(std::function<void()> callback)
{
    [static_cast<id>(handle) set_resize_callback: callback];
}

void Window::set_mouse_move_callback(std::function<void(int32_t, int32_t)> callback)
{
    [static_cast<id>(handle) set_mouse_move_callback: callback];
}

void Window::set_mouse_down_callback(std::function<void(MouseButton)> callback)
{
    [static_cast<id>(handle) set_mouse_down_callback: callback];
}

void Window::set_mouse_up_callback(std::function<void(MouseButton)> callback)
{
    [static_cast<id>(handle) set_mouse_up_callback: callback];
}

void Window::set_key_callback(std::function<void(const Key &)> callback)
{
    [static_cast<id>(handle)set_key_callback: callback];
}