#ifndef CG_SEM5_COURSEWORK_WINDOW_H
#define CG_SEM5_COURSEWORK_WINDOW_H

#include <functional>
#include <string>

#include "mousebutton.h"
#include "key.h"

class Window
{
public:
    struct Size
    {
        uint32_t width, height;
    };

    Window();
    Window(const std::string &title);

    ~Window();

    void show();
    void hide();

    void *get_view() const;

    void set_title(const std::string &);

    std::string get_title() const;

    void set_size(const Size &);

    Size get_size() const; 

    void set_view_size(const Size &);

    Size get_view_size() const;

    void set_resize_callback(std::function<void()>);

    void set_mouse_move_callback(std::function<void(int32_t, int32_t)>);

    void set_mouse_down_callback(std::function<void(MouseButton)>);

    void set_mouse_up_callback(std::function<void(MouseButton)>);

    void set_key_callback(std::function<void(const Key &)>);

private:
    void *handle;

    std::function<void()>                 resize_callback;
    std::function<void(int32_t, int32_t)> mouse_move_callback;
    std::function<void(MouseButton)>      mouse_down_callback;
    std::function<void(MouseButton)>      mouse_up_callback;
    std::function<void(const Key &)>      key_callback;
};

#endif // CG_SEM5_COURSEWORK_WINDOW_H