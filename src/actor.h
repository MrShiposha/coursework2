#ifndef CG_SEM5_ACTOR_H
#define CG_SEM5_ACTOR_H

#include <glm/glm.hpp>

#include "scenenode.h"

class Actor : public SceneNode
{
public:
    Actor();
    Actor(std::string_view id);

    const glm::mat4 &get_model_matrix() const;
    const glm::mat4 &model_matrix() const;
    glm::mat4 &model_matrix();
    void set_model_matrix(const glm::mat4 &);
    Actor &model_matrix(const glm::mat4 &);

    void translate(const glm::vec3 &);
    void rotate(const glm::vec3 &, const glm::vec3 &axis);

private:
    glm::mat4 model;
};

#endif // CG_SEM5_ACTOR_H