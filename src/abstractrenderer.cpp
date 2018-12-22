#include "abstractrenderer.h"

#include "window.h"

AbstractRenderer::AbstractRenderer(Window &window)
: window(window)
{
    window.set_resize_callback([this] { on_window_resize(); });
    window.set_mouse_move_callback([this](int32_t x, int32_t y) { on_mouse_move(x, y); });
    window.set_mouse_down_callback([this](MouseButton button)
    {
        if(button == MouseButton::LEFT)
            mouse_buttons.left = true;
        else
            mouse_buttons.right = true;

        on_mouse_down(button);
    });

    window.set_mouse_up_callback([this](MouseButton button)
    {
        if(button == MouseButton::LEFT)
            mouse_buttons.left = false;
        else
            mouse_buttons.right = false;

        on_mouse_up(button);
    });

    window.set_key_callback([this](const Key &key) { on_key(key); });
}

AbstractRenderer::~AbstractRenderer()
{}

void AbstractRenderer::on_window_resize()
{}

void AbstractRenderer::on_mouse_move(int32_t x, int32_t y)
{}

bool AbstractRenderer::is_left_mouse_button_pressed() const
{
    return mouse_buttons.left;
}

bool AbstractRenderer::is_left_mouse_button_released() const
{
    return !mouse_buttons.left;
}

bool AbstractRenderer::is_right_mouse_button_pressed() const
{
    return mouse_buttons.right;
}

bool AbstractRenderer::is_right_mouse_button_released() const
{
    return !mouse_buttons.right;
}

void AbstractRenderer::on_key(const Key &key)
{}

void AbstractRenderer::on_mouse_down(MouseButton button)
{}

void AbstractRenderer::on_mouse_up(MouseButton button)
{}