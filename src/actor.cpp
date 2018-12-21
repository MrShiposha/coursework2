#include "actor.h"

static size_t default_actor_id = 0;

Actor::Actor()
: SceneNode("actor" + std::to_string(default_actor_id++))
{}

Actor::Actor(std::string_view id)
: SceneNode(id)
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
}

Actor &Actor::model_matrix(const glm::mat4 &model)
{
    set_model_matrix(model);
    return *this;
}
