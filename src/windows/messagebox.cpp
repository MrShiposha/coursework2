#include <Windows.h>

#include "../messagebox.h"

void message_box(const std::string &title, const std::string &message)
{
    MessageBoxA(nullptr, message.c_str(), title.c_str(), MB_OK);
}