#ifndef CG_SEM5_OBJMESH_H
#define CG_SEM5_OBJMESH_H

#include <memory>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan.h>
#include <assimp/Importer.hpp> 
#include <assimp/scene.h>  
#include <assimp/postprocess.h>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include "abstractmesh.h"
#include "device.h"
#include "texture2d.h"

class StaticMesh : public AbstractMesh 
{
public:
    struct MaterialProperties
    {
        glm::vec4 ambient;
        glm::vec4 diffuse;
        glm::vec4 specular;
        float opacity;
    };

    struct Material
    {
        std::string                name;
        MaterialProperties         properties;
        std::shared_ptr<Texture2D> diffuse;

        VkDescriptorSet descriptor_set;
        VkPipeline     *pipeline;
    };

    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;
        glm::vec3 color;
    };

    struct Part
    {
        MeshElementIndex index_base;
        MeshElementIndex index_count;

        const Material *material;
    };


    static constexpr int DEFAULT_IMPORT_FLAGS = aiProcess_Triangulate 
                                              | aiProcess_PreTransformVertices
                                              | aiProcess_GenNormals;

    static std::shared_ptr<StaticMesh> load_from_file
    (
        std::string_view id, 
        std::string_view path,
        std::shared_ptr<Device>,
        VkCommandPool command_pool,
        VkQueue copy_queue,
        int import_flags = DEFAULT_IMPORT_FLAGS
    );

    virtual size_t get_vertex_count() const override;

    virtual const glm::vec3 &get_vertex_position(VertexIndex) const override;
    virtual const glm::vec2 &get_vertex_texture(VertexIndex) const override;
    virtual const glm::vec3 &get_vertex_normal(VertexIndex) const override;
    virtual const glm::vec3 &get_vertex_color(VertexIndex) const override;

    virtual const void *get_raw_vertices_data() const override;
    virtual const void *get_raw_indices_data() const override;

    const std::vector<Part> &get_parts() const;

private:
    StaticMesh
    (
        std::vector<Part> &&parts, 
        std::vector<Material> &&materials,
        std::vector<Vertex> &&vertices,
        std::vector<MeshElementIndex> &&indices
    );

    std::vector<Part>     parts;
    std::vector<Material> materials;

    std::vector<Vertex>   vertices;
    std::vector<MeshElementIndex> indices;
};

#endif // CG_SEM5_OBJMESH_H