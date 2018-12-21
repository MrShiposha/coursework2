#include "cameraselector.h"

CameraSelector::CameraSelector()
: current_camera(0)
{}

void CameraSelector::visit_up(std::shared_ptr<SceneNode>)
{}

void CameraSelector::visit_down(std::shared_ptr<SceneNode> node)
{
    if(auto camera = std::dynamic_pointer_cast<Camera>(node))
        cameras.push_back(camera);
}

void CameraSelector::reset_current_camera()
{
    current_camera = 0;
}

std::shared_ptr<Camera> CameraSelector::get_current_camera() const
{
    if(cameras.empty())
        throw std::runtime_error("Camera is not found");

    return cameras[current_camera];
}

std::shared_ptr<Camera> CameraSelector::get_next_camera()
{
    if(cameras.empty())
        throw std::runtime_error("Camera is not found");
    
    auto camera = cameras[current_camera];
    current_camera = (current_camera + 1) % cameras.size();

    return camera;
}