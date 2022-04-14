#include "model.hpp"

#include <ryujin/core/assets.hpp>

#include <tinygltf/tiny_gltf.h>

#undef APIENTRY
#include <spdlog/spdlog.h>

#include <utility>

namespace ryujin::assets
{
    static constexpr auto GLB_EXT = ".glb";
    static constexpr auto GLTF_EXT = ".gltf";
    static const std::string GLTF_POSITION_ACCESSOR = "POSITION";
    static const std::string GLTF_NORMAL_ACCESSOR = "NORMAL";
    static const std::string GLTF_TEXCOORD0_ACCESSOR = "TEXCOORD_0";
    static const std::string GLTF_TANGENT_ACCESSOR = "TANGENT";

    namespace gltf
    {
        const texture_asset* load_material_texture(const std::filesystem::path& path, tinygltf::Model model, tinygltf::TextureInfo tex, asset_manager* manager)
        {
            if (tex.index < 0)
            {
                return nullptr;
            }

            auto index = model.textures[tex.index].source;
            auto source = model.images[index];

            if (source.uri.empty())
            {
                spdlog::warn("Loading texture from memory not handled");
                return nullptr;
            }
            else
            {
                return manager->load_texture(path / source.uri, false);
            }
        }

        const texture_asset* load_material_texture(const std::filesystem::path& path, tinygltf::Model model, tinygltf::NormalTextureInfo tex, asset_manager* manager)
        {
            if (tex.index < 0)
            {
                return nullptr;
            }

            auto index = model.textures[tex.index].source;
            auto source = model.images[index];

            if (source.uri.empty())
            {
                spdlog::warn("Loading texture from memory not handled");
                return nullptr;
            }
            else
            {
                return manager->load_texture(path / source.uri, false);
            }
        }

        const texture_asset* load_material_texture(const std::filesystem::path& path, tinygltf::Model model, tinygltf::OcclusionTextureInfo tex, asset_manager* manager)
        {
            if (tex.index < 0)
            {
                return nullptr;
            }

            auto index = model.textures[tex.index].source;
            auto source = model.images[index];

            if (source.uri.empty())
            {
                spdlog::warn("Loading texture from memory not handled");
                return nullptr;
            }
            else
            {
                return manager->load_texture(path / source.uri, false);
            }
        }

        vector<mesh_group> load_meshes(tinygltf::Model& gltfModel, const std::string& path, asset_manager* manager)
        {
            auto parent = std::filesystem::path(path).parent_path();

            vector<const material_asset*> materials;
            for (const auto& gltfMaterial : gltfModel.materials)
            {
                auto baseInfo = gltfMaterial.pbrMetallicRoughness.baseColorTexture;
                auto normalInfo = gltfMaterial.normalTexture;
                auto occlussionInfo = gltfMaterial.occlusionTexture;
                auto emissiveInfo = gltfMaterial.emissiveTexture;
                auto metalRoughInfo = gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture;
                auto alphaMode = gltfMaterial.alphaMode == "OPAQUE" ? alpha_mode::OPAQUE : alpha_mode::BLENDED;
                
                auto baseColorTexture = load_material_texture(parent, gltfModel, baseInfo, manager);
                auto normalMapTexture = load_material_texture(parent, gltfModel, normalInfo, manager);
                auto occlusionTexture = load_material_texture(parent, gltfModel, occlussionInfo, manager);
                auto emmissiveTexture = load_material_texture(parent, gltfModel, emissiveInfo, manager);
                auto metalRoughTexture = load_material_texture(parent, gltfModel, metalRoughInfo, manager);
            
                material_asset mat = {
                    .name = gltfMaterial.name,
                    .baseColorTexture = baseColorTexture,
                    .normalTexture = normalMapTexture,
                    .occlusionTexture = occlusionTexture,
                    .emissiveTexture = emmissiveTexture,
                    .metallicRoughness = metalRoughTexture,
                    .alpha = alphaMode
                };

                auto asset = manager->load_material(mat.name, mat);
                materials.push_back(asset);
            }

            vector<mesh_group> meshes;

            for (const auto& gltfMesh : gltfModel.meshes)
            {
                mesh_group group;
                int primitiveCount = 0;
                for (const auto& prim : gltfMesh.primitives)
                {
                    mesh m;
                    m.name = fmt::v8::format("{}_{}", gltfMesh.name, primitiveCount++);

                    // populate mesh vertices
                    const auto drawMode = prim.mode;
                    const auto indicesAccessorIndex = prim.indices;
                    const auto positionsAccessorIterator = prim.attributes.find(GLTF_POSITION_ACCESSOR);
                    const auto normalsAccessorIterator = prim.attributes.find(GLTF_NORMAL_ACCESSOR);
                    const auto textureUvsAccessorIterator = prim.attributes.find(GLTF_TEXCOORD0_ACCESSOR);
                    const auto tangentAccessorIterator = prim.attributes.find(GLTF_TANGENT_ACCESSOR);

                    const auto endAccessorIterator = prim.attributes.end();

                    if (drawMode != TINYGLTF_MODE_TRIANGLES)
                    {
                        // TODO: support other draw modes
                        spdlog::error("Unsupported draw mode {} for mesh {} in file {}.", drawMode, m.name, path);
                        return {};
                    }

                    if (indicesAccessorIndex < 0)
                    {
                        spdlog::error("Mesh {} in file {} does not have indices.", m.name, path);
                        return {};
                    }
                    else
                    {
                        auto& accessor = gltfModel.accessors[indicesAccessorIndex];
                        auto& bufferView = gltfModel.bufferViews[accessor.bufferView];
                        auto& buffer = gltfModel.buffers[bufferView.buffer];
                        auto data = buffer.data.data();

                        if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
                        {
                            for (std::size_t i = 0; i < accessor.count; ++i)
                            {
                                auto dataOffset = bufferView.byteOffset + accessor.byteOffset + i * accessor.ByteStride(bufferView);
                                auto rawStart = data + dataOffset;
                                unsigned int* start = reinterpret_cast<unsigned int*>(rawStart);
                                m.indices[i] = *start;
                            }
                        }
                        else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                        {
                            for (std::size_t i = 0; i < accessor.count; ++i)
                            {
                                auto dataOffset = bufferView.byteOffset + accessor.byteOffset + i * accessor.ByteStride(bufferView);
                                auto rawStart = data + dataOffset;
                                unsigned char* start = reinterpret_cast<unsigned char*>(rawStart);

                                m.indices[i] = *start;
                            }
                        }
                        else
                        {

                        }
                    }

                    if (positionsAccessorIterator == endAccessorIterator)
                    {
                        spdlog::error("Mesh {} in file {} does not have positions.", m.name, path);
                        return {};
                    }
                    else
                    {
                        auto& [name, positionAccessorIndex] = *positionsAccessorIterator;
                        auto& accessor = gltfModel.accessors[positionAccessorIndex];
                        auto& bufferView = gltfModel.bufferViews[accessor.bufferView];
                        auto& buffer = gltfModel.buffers[bufferView.buffer];
                        auto data = buffer.data.data();

                        m.vertices.resize(accessor.count);
                        
                        for (std::size_t i = 0; i < accessor.count; ++i)
                        {
                            auto dataOffset = bufferView.byteOffset + accessor.byteOffset + i * accessor.ByteStride(bufferView);
                            auto rawStart = data + dataOffset;
                            float* start = reinterpret_cast<float*>(rawStart);

                            auto& v = m.vertices[i];

                            v.position.x = start[0];
                            v.position.y = start[1];
                            v.position.z = start[2];
                        }
                    }

                    if (normalsAccessorIterator != endAccessorIterator)
                    {
                        auto& [name, normalsAccessorIndex] = *normalsAccessorIterator;
                        auto& accessor = gltfModel.accessors[normalsAccessorIndex];
                        auto& bufferView = gltfModel.bufferViews[accessor.bufferView];
                        auto& buffer = gltfModel.buffers[bufferView.buffer];
                        auto data = buffer.data.data();
                        
                        for (std::size_t i = 0; i < accessor.count; ++i)
                        {
                            auto dataOffset = bufferView.byteOffset + accessor.byteOffset + i * accessor.ByteStride(bufferView);
                            auto rawStart = data + dataOffset;
                            float* start = reinterpret_cast<float*>(rawStart);

                            auto& v = m.vertices[i];

                            v.normal.x = start[0];
                            v.normal.y = start[1];
                            v.normal.z = start[2];
                        }
                    }

                    if (textureUvsAccessorIterator != endAccessorIterator)
                    {
                        auto& [name, texcoord0AccessorIndex] = *textureUvsAccessorIterator;
                        auto& accessor = gltfModel.accessors[texcoord0AccessorIndex];
                        auto& bufferView = gltfModel.bufferViews[accessor.bufferView];
                        auto& buffer = gltfModel.buffers[bufferView.buffer];
                        auto data = buffer.data.data();

                        if (accessor.type == TINYGLTF_COMPONENT_TYPE_FLOAT)
                        {
                            for (std::size_t i = 0; i < accessor.count; ++i)
                            {
                                auto dataOffset = bufferView.byteOffset + accessor.byteOffset + i * accessor.ByteStride(bufferView);
                                auto rawStart = data + dataOffset;
                                float* start = reinterpret_cast<float*>(rawStart);

                                auto& v = m.vertices[i];

                                v.texCoord.x = start[0];
                                v.texCoord.y = start[1];
                            }
                        }
                        else if (accessor.type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
                        {
                            for (std::size_t i = 0; i < accessor.count; ++i)
                            {
                                auto dataOffset = bufferView.byteOffset + accessor.byteOffset + i * accessor.ByteStride(bufferView);
                                auto rawStart = data + dataOffset;
                                unsigned char* start = reinterpret_cast<unsigned char*>(rawStart);

                                auto& v = m.vertices[i];

                                v.texCoord.x = as<float>(start[0]) / as<float>(UINT8_MAX);
                                v.texCoord.y = as<float>(start[1]) / as<float>(UINT8_MAX);
                            }
                        }
                        else if (accessor.type == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
                        {
                            for (std::size_t i = 0; i < accessor.count; ++i)
                            {
                                auto dataOffset = bufferView.byteOffset + accessor.byteOffset + i * accessor.ByteStride(bufferView);
                                auto rawStart = data + dataOffset;
                                unsigned short* start = reinterpret_cast<unsigned short*>(rawStart);

                                auto& v = m.vertices[i];

                                v.texCoord.x = as<float>(start[0]) / as<float>(UINT16_MAX);
                                v.texCoord.y = as<float>(start[1]) / as<float>(UINT16_MAX);
                            }
                        }
                    }

                    if (tangentAccessorIterator != endAccessorIterator)
                    {
                        auto& [name, tangentAccessIndex] = *tangentAccessorIterator;
                        auto& accessor = gltfModel.accessors[tangentAccessIndex];
                        auto& bufferView = gltfModel.bufferViews[accessor.bufferView];
                        auto& buffer = gltfModel.buffers[bufferView.buffer];
                        auto data = buffer.data.data();
                        
                        for (std::size_t i = 0; i < accessor.count; ++i)
                        {
                            auto dataOffset = bufferView.byteOffset + accessor.byteOffset + i * accessor.ByteStride(bufferView);
                            auto rawStart = data + dataOffset;
                            float* start = reinterpret_cast<float*>(rawStart);

                            auto& v = m.vertices[i];

                            v.tangent.x = start[0];
                            v.tangent.y = start[1];
                            v.tangent.z = start[2];
                            v.tangent.w = start[3];
                        }
                    }

                    // compute mesh materials
                    m.material = materials[prim.material];
                    group.meshes.push_back(m);
                }
                meshes.push_back(std::move(group));
            }
            
            return meshes;
        }
    }

    vector<std::unique_ptr<model_asset>> load_model(const std::string& path, const std::string& ext, asset_manager* manager)
    {
        vector<std::unique_ptr<model_asset>> models;
        
        if (ext == GLB_EXT || ext == GLTF_EXT)
        {
            using tinygltf::Model;
            using tinygltf::TinyGLTF;

            TinyGLTF gltfLoader;
            Model gltfModel;

            std::string err, warn;
            const bool ret = ext == GLB_EXT ? gltfLoader.LoadBinaryFromFile(&gltfModel, &err, &warn, path) : gltfLoader.LoadASCIIFromFile(&gltfModel, &err, &warn, path);
            if (!ret)
            {
                spdlog::error("Failed to load GLTF model {}: {}", path, err);
                return {};
            }

            vector<mesh_group> meshes = gltf::load_meshes(gltfModel, path, manager);
            vector<slot_map_key> meshKeys;
            meshKeys.reserve(meshes.size());

            for (auto& group : meshes)
            {
                meshKeys.push_back(manager->load_mesh_group(group));
            }

            models.reserve(meshes.size());

            for (auto node : gltfModel.nodes)
            {
                
            }
        }
        
        return models;
    }
}