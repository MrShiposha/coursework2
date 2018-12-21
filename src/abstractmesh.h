#ifndef CG_SEM5_COURSEWORK_MESH_H
#define CG_SEM5_COURSEWORK_MESH_H

#include <vector>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>

#include "actor.h"

using MeshElementIndex   = uint32_t;
using VertexIndex        = MeshElementIndex;
using VertexTextureIndex = MeshElementIndex;
using VertexNormalIndex  = MeshElementIndex;
using FaceIndex          = MeshElementIndex;
using Face               = std::vector<MeshElementIndex>;

class AbstractMesh : public Actor
{
public:
    AbstractMesh();
    AbstractMesh(std::string_view id);

    virtual size_t get_vertex_count() const = 0;

    virtual const glm::vec3 &get_vertex_position(VertexIndex) const = 0;
    virtual const glm::vec2 &get_vertex_texture(VertexIndex) const = 0;
    virtual const glm::vec3 &get_vertex_normal(VertexIndex) const = 0;
    virtual const glm::vec3 &get_vertex_color(VertexIndex) const = 0;

    virtual const void *get_raw_vertices_data() const = 0;
    virtual const void *get_raw_indices_data() const = 0;
};

#endif // CG_SEM5_COURSEWORK_MESH_H