#include "actorscontainer.h"

void ActorsContainer::visit_up(std::shared_ptr<SceneNode>)
{}

void ActorsContainer::visit_down(std::shared_ptr<SceneNode> node)
{
    if(auto actor = std::dynamic_pointer_cast<Actor>(node))
        actors.push_back(actor);
}

const std::vector<std::shared_ptr<Actor>> &ActorsContainer::get_actors() const
{
    return actors;
}