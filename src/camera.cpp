#include "camera.h"

static size_t default_camera_id = 0;

Camera::Camera(float fov, float aspect_ratio, float znear, float zfar)
: Camera("camera" + std::to_string(default_camera_id++), fov, aspect_ratio, znear, zfar)
{}

Camera::Camera(std::string_view id, float fov, float aspect_ratio, float znear, float zfar)
: Actor(id),
fov(fov),
aspect_ratio(aspect_ratio),
znear(znear),
zfar(zfar)
{}

float Camera::get_fov() const
{
    return fov;
}

float Camera::get_aspect_ratio() const
{
    return aspect_ratio;
}

float Camera::get_znear() const
{
    return znear;
}

float Camera::get_zfar() const
{
    return zfar;
}
