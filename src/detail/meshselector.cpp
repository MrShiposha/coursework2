#include "meshselector.h"

MeshSelector::MeshSelector()
: current_mesh(0)
{}

void MeshSelector::visit_up(std::shared_ptr<SceneNode>)
{}

void MeshSelector::visit_down(std::shared_ptr<SceneNode> node)
{
    if(auto mesh = std::dynamic_pointer_cast<AbstractMesh>(node))
        meshes.push_back(mesh);
}

void MeshSelector::reset_current_mesh()
{
    current_mesh = 0;
}

std::shared_ptr<AbstractMesh> MeshSelector::get_next_mesh()
{
    if(meshes.empty())
        return nullptr;
    
    auto mesh = meshes[current_mesh];
    current_mesh = (current_mesh + 1) % meshes.size();

    return mesh;
}