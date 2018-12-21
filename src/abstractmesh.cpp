#include "abstractmesh.h"

static size_t default_mesh_id = 0;

AbstractMesh::AbstractMesh()
: Actor("mesh" + std::to_string(default_mesh_id++))
{}

AbstractMesh::AbstractMesh(std::string_view id)
: Actor(id)
{}