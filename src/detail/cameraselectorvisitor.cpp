#include "cameraselectorvisitor.h"

CameraSelectorVisitor::CameraSelectorVisitor()
: current_camera(0)
{}

void CameraSelectorVisitor::visit_up(std::shared_ptr<SceneNode>)
{}

void CameraSelectorVisitor::visit_down(std::shared_ptr<SceneNode> node)
{
    if(auto camera = std::dynamic_pointer_cast<Camera>(node))
        cameras.push_back(node);
}

void CameraSelectorVisitor::reset_current_camera()
{
    current_camera = 0;
}

std::shared_ptr<Camera> CameraSelectorVisitor::get_next_camera()
{
    if(cameras.empty())
        throw std::runtime_error("Camera is not found");
    
    auto camera = cameras[current_camera];
    current_camera = (current_camera + 1) % cameras.size();

    return camera;
}