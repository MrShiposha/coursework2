#include "materialcountervisitor.h"

#include "../staticmesh.h"

MaterialCounterVisitor::MaterialCounterVisitor()
: materials_count(0)
{}

uint32_t MaterialCounterVisitor::get_materials_count() const
{
    return materials_count;
}

void MaterialCounterVisitor::visit_up(SceneNode &)
{}

void MaterialCounterVisitor::visit_down(SceneNode &node)
{
    if(auto *static_mesh = dynamic_cast<StaticMesh*>(&node))
        materials_count += static_cast<uint32_t>(static_mesh->get_materials().size());
}