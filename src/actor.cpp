#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>

#include "actor.h"

static size_t default_actor_id = 0;

Actor::Actor()
: Actor("actor" + std::to_string(default_actor_id++))
{}

Actor::Actor(std::string_view id)
: SceneNode(id), model(glm::mat4(1.f))
{}

const glm::mat4 &Actor::get_model_matrix() const
{
    return model_matrix();
}

const glm::mat4 &Actor::model_matrix() const
{
    return const_cast<Actor*>(this)->model_matrix();
}

glm::mat4 &Actor::model_matrix()
{
    return model;
}

void Actor::set_model_matrix(const glm::mat4 &model)
{
    this->model = model;
    this->changed = true;
}

Actor &Actor::model_matrix(const glm::mat4 &model)
{
    set_model_matrix(model);
    return *this;
}

void Actor::translate(const glm::vec3 &translation)
{
    model = glm::translate(model, translation);
    this->changed = true;
}

void Actor::rotate(float angle, const glm::vec3 &axis)
{
    model = glm::rotate(model, angle, axis);
    this->changed = true;
}

void Actor::scale(const glm::vec3 &scale)
{
    model = glm::scale(model, scale);
    this->changed = true;
}