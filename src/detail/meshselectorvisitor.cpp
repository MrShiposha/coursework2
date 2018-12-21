#include "meshselectorvisitor.h"

MeshSelectorVisitor::MeshSelectorVisitor()
: current_mesh(0)
{}

void MeshSelectorVisitor::visit_up(std::shared_ptr<SceneNode>)
{}

void MeshSelectorVisitor::visit_down(std::shared_ptr<SceneNode> node)
{
    if(auto mesh = std::dynamic_pointer_cast<AbstractMesh>(node))
        meshes.push_back(node);
}

void MeshSelectorVisitor::reset_current_mesh()
{
    current_mesh = 0;
}

std::shared_ptr<AbstractMesh> MeshSelectorVisitor::get_next_mesh()
{
    if(meshes.empty())
        return nullptr;
    
    auto mesh = meshes[current_camera];
    current_mesh = (current_mesh + 1) % meshes.size();

    return meshes;
}