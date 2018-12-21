#ifndef CG_SEM5_CAMERASELECTORVISITOR_H
#define CG_SEM5_CAMERASELECTORVISITOR_H

#include <stdexcept>
#include <cstdint>
#include <cstddef>
#include <vector>

#include "../scenegraphvisitor.h"
#include "../camera.h"

class CameraSelectorVisitor : public SceneGraphVisitor
{
public:
    CameraSelectorVisitor();

    virtual void visit_up(std::shared_ptr<SceneNode>) override;
    virtual void visit_down(std::shared_ptr<SceneNode>) override;

    void reset_current_camera();
    std::shared_ptr<Camera> get_next_camera();

private:
    size_t current_camera;
    std::vector<Camera*> cameras;
};

#endif // CG_SEM5_CAMERASELECTORVISITOR_H