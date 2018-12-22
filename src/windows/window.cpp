#include <Windows.h>
#include <memory>
#include <stdexcept>

#include "../window.h"

constexpr float DEFAULT_WIDTH  = 640;
constexpr float DEFAULT_HEIGHT = 500;

std::shared_ptr<WNDCLASSEXA> wndclass;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_CLOSE:
            DestroyWindow(hwnd);
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
        break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

Window::Window()
: handle(nullptr)
{
    static char *classname = "courseworkwindowclass";

    if(wndclass == nullptr)
    {
        wndclass = std::make_shared<WNDCLASSEX>();
        wndclass->cbSize        = sizeof(WNDCLASSEX);
        wndclass->style         = 0;
        wndclass->lpfnWndProc   = WndProc;
        wndclass->cbClsExtra    = 0;
        wndclass->cbWndExtra    = 0;
        wndclass->hInstance     = GetModuleHandle(nullptr);
        wndclass->hIcon         = LoadIcon(NULL, IDI_APPLICATION);
        wndclass->hCursor       = LoadCursor(NULL, IDC_ARROW);
        wndclass->hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
        wndclass->lpszMenuName  = NULL;
        wndclass->lpszClassName = classname;
        wndclass->hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

        if(!RegisterClassExA(wndclass.get()))
            throw std::runtime_error("Can't register window class");
    }

    handle = CreateWindowExA
    (
        WS_EX_APPWINDOW,
        classname,
        "",
        WS_CAPTION | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT,
        static_cast<int>(DEFAULT_WIDTH), static_cast<int>(DEFAULT_HEIGHT),
        nullptr, nullptr, GetModuleHandle(nullptr), nullptr
    );

    if(handle == nullptr)
        throw std::runtime_error("Can't create window");
}

Window::Window(const std::string &title)
: Window()
{
    set_title(title);
}

Window::~Window()
{
}

void *Window::get_view() const
{
    return handle;
}

void Window::set_title(const std::string &title)
{
    SetWindowTextA(static_cast<HWND>(handle), title.c_str());
}

std::string Window::get_title() const
{
    constexpr size_t SIZE = 512;
    char text[SIZE];
    GetWindowTextA(static_cast<HWND>(handle), text, SIZE);

    return text;
}

void Window::set_size(const Size &size)
{
    SetWindowPos(static_cast<HWND>(handle), HWND_TOP, CW_USEDEFAULT, CW_USEDEFAULT, size.width, size.height, SWP_FRAMECHANGED | SWP_SHOWWINDOW);
}

Window::Size Window::get_size() const
{
    RECT rect;
    GetWindowRect(static_cast<HWND>(handle), &rect);

    return Size { static_cast<uint32_t>(rect.right - rect.left), static_cast<uint32_t>(rect.top - rect.bottom) };
}

void Window::set_view_size(const Size &size)
{
    RECT rect;
    rect.top = 0;
    rect.left = 0;
    rect.right = size.width;
    rect.bottom = size.height;

    AdjustWindowRectEx(&rect, WS_POPUP, false, WS_EX_APPWINDOW);
}

Window::Size Window::get_view_size() const
{
    RECT rect;
    GetClientRect(static_cast<HWND>(handle), &rect);

    return Size { static_cast<uint32_t>(rect.right - rect.left), static_cast<uint32_t>(rect.bottom - rect.top) };
}

void Window::show()
{
    ShowWindow(static_cast<HWND>(handle), SW_SHOW);
    UpdateWindow(static_cast<HWND>(handle));
}

void Window::hide()
{
    ShowWindow(static_cast<HWND>(handle), SW_HIDE);
}

void Window::set_resize_callback(std::function<void()> callback)
{
    resize_callback = callback;
}

void Window::set_mouse_move_callback(std::function<void(int32_t, int32_t)> callback)
{
    mouse_move_callback = callback;
}

void Window::set_mouse_down_callback(std::function<void(MouseButton)> callback)
{
    mouse_down_callback = callback;
}

void Window::set_mouse_up_callback(std::function<void(MouseButton)> callback)
{
    mouse_up_callback = callback;    
}

void Window::set_key_callback(std::function<void(const Key &)> callback)
{
    key_callback = callback;
}