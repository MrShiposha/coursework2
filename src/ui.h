#ifndef CG_SEM5_COURSEWORK_UI_H
#define CG_SEM5_COURSEWORK_UI_H

#include <vector>

class AbstractRenderer;

class Ui
{
public:
    static void execute();

    static void stop();

    static void register_renderer(AbstractRenderer &);

    static void render_all();

private:
    static bool is_stopped;
    static std::vector<AbstractRenderer*> renderers;

};

#endif // CG_SEM5_COURSEWORK_UI_H