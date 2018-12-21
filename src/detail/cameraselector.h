#ifndef CG_SEM5_CAMERASELECTOR_H
#define CG_SEM5_CAMERASELECTOR_H

#include <stdexcept>
#include <cstdint>
#include <cstddef>
#include <vector>

#include "../scenegraphvisitor.h"
#include "../camera.h"

class CameraSelector : public SceneGraphVisitor
{
public:
    CameraSelector();

    virtual void visit_up(std::shared_ptr<SceneNode>) override;
    virtual void visit_down(std::shared_ptr<SceneNode>) override;

    void reset_current_camera();
    std::shared_ptr<Camera> get_current_camera() const;
    std::shared_ptr<Camera> get_next_camera();

private:
    size_t current_camera;
    std::vector<std::shared_ptr<Camera>> cameras;
};

#endif // CG_SEM5_CAMERASELECTOR_H