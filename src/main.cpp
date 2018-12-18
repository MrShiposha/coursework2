#include <stdexcept>

#include "initialize.h"
#include "ui.h"
#include "window.h"
#include "messagebox.h"

int main() try
{
    initialize();

    Window window;

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