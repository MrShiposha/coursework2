#ifndef CG_SEM5_COURSEWORK_ASBTRACTRENDERER_H
#define CG_SEM5_COURSEWORK_ASBTRACTRENDERER_H

#include <cstdint>

#include "window.h"

class AbstractRenderer
{
    struct MouseButtons
    {
        bool left;
        bool right;
    };
public:
    AbstractRenderer(Window &);

    virtual ~AbstractRenderer();

    virtual void render() = 0;

    virtual void on_window_resize();

    virtual void on_mouse_move(int32_t x, int32_t y);

    bool is_left_mouse_button_pressed() const;

    bool is_left_mouse_button_released() const;

    bool is_right_mouse_button_pressed() const;

    bool is_right_mouse_button_released() const;

protected:
    Window &window;
    MouseButtons mouse_buttons;

};

#endif // CG_SEM5_COURSEWORK_ASBTRACTRENDERER_H