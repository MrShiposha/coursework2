#include "staticmeshescontainer.h"

const std::vector<std::shared_ptr<StaticMesh>> &StaticMeshesContainer::get_meshes() const
{
    return meshes;
}

void StaticMeshesContainer::visit_up(std::shared_ptr<SceneNode>)
{}

void StaticMeshesContainer::visit_down(std::shared_ptr<SceneNode> node)
{
    if(auto static_mesh = std::dynamic_pointer_cast<StaticMesh>(node))
        meshes.push_back(static_mesh);
}