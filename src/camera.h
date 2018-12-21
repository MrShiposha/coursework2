#ifndef CG_SEM5_CAMERA_H
#define CG_SEM5_CAMERA_H

#include "actor.h"

class Camera : public Actor
{
public:
    Camera(float fov, float aspect_ratio, float znear, float zfar);
    Camera(std::string_view id, float fov, float aspect_ratio, float znear, float zfar);

    const glm::mat4 &get_perspective_matrix() const;
    float get_fov() const;
    float get_aspect_ratio() const;
    float get_znear() const;
    float get_zfar() const;

protected:
    float fov;
    float aspect_ratio;
    float znear, zfar;

    glm::mat4 perspective;
};

#endif // CG_SEM5_CAMERA_H