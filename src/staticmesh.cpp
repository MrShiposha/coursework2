#include <glm/gtc/type_ptr.hpp>

#include "staticmesh.h"

void load_materials
(
    const aiScene *,
    std::shared_ptr<Device> device,
    VkCommandPool command_pool,
    VkQueue copy_queue,
    std::vector<StaticMesh::Material> &
);

void load_parts
(
    const aiScene *,
    const std::vector<StaticMesh::Material> &,
    std::vector<StaticMesh::Part> &, 
    std::vector<StaticMesh::Vertex> &, 
    std::vector<MeshElementIndex> &
);

std::shared_ptr<StaticMesh> StaticMesh::load_from_file
(
    std::string_view id, 
    std::string_view path, 
    std::shared_ptr<Device> device,
    VkCommandPool command_pool,
    VkQueue copy_queue,
    int import_flags
)
{
    using namespace std::string_literals;

    std::vector<Part>             parts;
    std::vector<Material>         materials;
    std::vector<Vertex>           vertices;
    std::vector<MeshElementIndex> indices;


    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path.data(), import_flags);

    if(scene == nullptr)
        throw std::runtime_error("Can't load mesh from file \""s + path.data() + "\"");

    load_materials(scene, device, command_pool, copy_queue, materials);
    load_parts(scene, materials, parts, vertices, indices);

    return std::shared_ptr<StaticMesh>
    (
        new StaticMesh
        (
            std::move(parts),
            std::move(materials),
            std::move(vertices),
            std::move(indices)
        )
    );
}

StaticMesh::StaticMesh
(
    std::vector<Part> &&parts, 
    std::vector<Material> &&materials,
    std::vector<Vertex> &&vertices,
    std::vector<MeshElementIndex> &&indices
) : parts(std::forward<std::vector<Part>>(parts)),
materials(std::forward<std::vector<Material>>(materials)),
vertices(std::forward<std::vector<Vertex>>(vertices)),
indices(std::forward<std::vector<MeshElementIndex>>(indices))
{}

size_t StaticMesh::get_vertex_count() const
{
    return vertices.size();
}

const glm::vec3 &StaticMesh::get_vertex_position(VertexIndex index) const
{
    return vertices.at(index).position;
}

const glm::vec2 &StaticMesh::get_vertex_texture(VertexIndex index) const
{
    return vertices.at(index).uv;
}

const glm::vec3 &StaticMesh::get_vertex_normal(VertexIndex index) const
{
    return vertices.at(index).normal;
}

const glm::vec3 &StaticMesh::get_vertex_color(VertexIndex index) const
{
    return vertices.at(index).color;
}

const void *StaticMesh::get_raw_vertices_data() const
{
    return static_cast<const void*>(vertices.data());
}

const void *StaticMesh::get_raw_indices_data() const
{
    return static_cast<const void*>(indices.data());
}

const std::vector<StaticMesh::Vertex> &StaticMesh::get_vertices() const
{
    return vertices;
}

const std::vector<MeshElementIndex> &StaticMesh::get_indices() const
{
    return indices;
}

const std::vector<StaticMesh::Part> &StaticMesh::get_parts() const
{
    return parts;
}

const std::vector<StaticMesh::Material> &StaticMesh::get_materials() const
{
    return materials;
}

void load_materials
(
    const aiScene *scene,
    std::shared_ptr<Device> device,
    VkCommandPool command_pool,
    VkQueue copy_queue,
    std::vector<StaticMesh::Material> &materials
)
{
    materials.resize(scene->mNumMaterials);
    for(size_t i = 0; i < materials.size(); ++i)
    {
        materials[i] = { 0 };

        aiString material_name;
        scene->mMaterials[i]->Get(AI_MATKEY_NAME, material_name);
        materials[i].name = material_name.C_Str();

        aiColor4D color;
        scene->mMaterials[i]->Get(AI_MATKEY_COLOR_AMBIENT, color);
        materials[i].properties.ambient = glm::make_vec4(&color.r) + glm::vec4(0.1f);

        scene->mMaterials[i]->Get(AI_MATKEY_COLOR_DIFFUSE, color);
        materials[i].properties.diffuse = glm::make_vec4(&color.r);

        scene->mMaterials[i]->Get(AI_MATKEY_COLOR_SPECULAR, color);
        materials[i].properties.specular = glm::make_vec4(&color.r);

        scene->mMaterials[i]->Get(AI_MATKEY_OPACITY, materials[i].properties.opacity);

        if(materials[i].properties.opacity > 0.f)
            materials[i].properties.specular = glm::vec4(0.f);

        VkFormat texture_format;
        std::string texture_format_suffix;
        if(device->features.textureCompressionBC)
        {
            texture_format = VK_FORMAT_BC3_UNORM_BLOCK;
            texture_format_suffix = "_bc3_unorm";
        }
        else if(device->features.textureCompressionASTC_LDR)
        {
            texture_format = VK_FORMAT_ASTC_8x8_UNORM_BLOCK;
            texture_format_suffix = "_astc_8x8_unorm";
        }
        else if(device->features.textureCompressionETC2)
        {
            texture_format = VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK;
            texture_format_suffix = "_etc2_unorm";
        }
        else
            throw std::runtime_error("Device does not support any compressed texture format");

        if(scene->mMaterials[i]->GetTextureCount(aiTextureType_DIFFUSE) < 1)
            throw std::runtime_error("Textures not found");

        aiString texture_file;
        scene->mMaterials[i]->GetTexture(aiTextureType_DIFFUSE, 0, &texture_file);
        std::string compressed_texture_file = texture_file.C_Str();
        compressed_texture_file.insert(compressed_texture_file.find(".ktx"), texture_format_suffix); // !!!
        materials[i].diffuse = Texture2D::load_from_file(compressed_texture_file, texture_format, device, command_pool, copy_queue);
    }
}

void load_parts
(
    const aiScene *scene,
    const std::vector<StaticMesh::Material> &materials,
    std::vector<StaticMesh::Part> &parts, 
    std::vector<StaticMesh::Vertex> &vertices, 
    std::vector<MeshElementIndex> &indices
)
{
    static constexpr MeshElementIndex FACE_ELEMENT_COUNT = 3;
    MeshElementIndex index_base = 0;

    parts.resize(scene->mNumMeshes);

    StaticMesh::Vertex vertex;
    bool has_uv      = false;
    bool has_color   = false;
    bool has_normals = false;
    aiMesh *mesh = nullptr;

    for(MeshElementIndex i = 0, j = 0, v = 0, f = 0; i < scene->mNumMeshes; ++i)
    {
        mesh = scene->mMeshes[i];

        parts[i].material    = &materials[mesh->mMaterialIndex];
        parts[i].index_base  = index_base;
        parts[i].index_count = mesh->mNumFaces * FACE_ELEMENT_COUNT;

        has_uv      = mesh->HasTextureCoords(0);
        has_color   = mesh->HasVertexColors(0);
        has_normals = mesh->HasNormals();

        for(v = 0; v < mesh->mNumVertices; ++v)
        {
            vertex.position = glm::make_vec3(&mesh->mVertices[v].x); // Taking address of vector begin
            vertex.position.y *= 1; // Flip y for Vulkan

            vertex.uv     = has_uv? glm::make_vec2(&mesh->mTextureCoords[0][v].x) : glm::vec2(0.f);
            
            vertex.normal = has_normals? glm::make_vec3(&mesh->mNormals[v].x) : glm::vec3(0.f);
            vertex.normal.y *= -1;

            vertex.color = has_color? glm::make_vec3(&mesh->mColors[0][v].r) : glm::vec3(1.f);
            vertices.push_back(vertex);
        }

        for(f = 0; f < mesh->mNumFaces; ++f)
            for(j = 0; j < FACE_ELEMENT_COUNT; ++j) indices.push_back(mesh->mFaces[f].mIndices[j]);

        index_base += mesh->mNumFaces * FACE_ELEMENT_COUNT;
    }
}