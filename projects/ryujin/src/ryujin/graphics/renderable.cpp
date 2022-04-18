#include <ryujin/graphics/renderable.hpp>

#include <ryujin/core/as.hpp>
#include <ryujin/graphics/camera_component.hpp>
#include <ryujin/graphics/render_manager.hpp>

#undef APIENTRY
#include <spdlog/spdlog.h>

namespace ryujin
{
    renderable_manager::renderable_manager(render_manager* manager, registry* reg)
        : _registry(reg), _manager(manager)
    {
        const sampler_create_info info = {
            .min = filter::LINEAR,
            .mag = filter::LINEAR,
            .mipmapMode = mipmap_mode::LINEAR,
            .u = address_mode::REPEAT,
            .v = address_mode::REPEAT,
            .w = address_mode::REPEAT,
            .mipLodBias = 0.0f,
            .enableAnisotropy = false,
            .maxAnisotropy = 0.0f,
            .compare = std::nullopt,
            .minLod = 0.0f,
            .maxLod = 0.0f,
            .unnormalizedCoordinates = false,
            .name = "default_texture_sampler"
        };

        auto samplerResult = _manager->create(info);
        if (samplerResult)
        {
            _defaultSampler = *samplerResult;
        }
        else
        {
            spdlog::error("Failed to create default texture sampler.");
        }

        const std::function renderableAddedCallback = [this](const component_add_event<renderable_component, registry::entity_type>& e) {
            register_entity(e.entity.handle());
        };

        const std::function renderableRemovedCallback = [this](const component_remove_event<renderable_component, registry::entity_type>& e) {
            unregister_entity(e.entity.handle());
        };

        const std::function renderableReplacedCallback = [this](const component_replace_event<renderable_component, registry::entity_type>& e) {
            update_entity(e.entity.handle());
        };

        reg->events().subscribe(renderableAddedCallback);
        reg->events().subscribe(renderableRemovedCallback);
        reg->events().subscribe(renderableReplacedCallback);

        const std::function cameraCreatedCallback = [this](const component_add_event<camera_component, registry::entity_type>& e) {
            register_camera(e.entity.handle());
        };

        const std::function cameraDestroyedCallback = [this](const component_remove_event<camera_component, registry::entity_type>& e) {
            unregister_camera(e.entity.handle());
        };

        reg->events().subscribe(cameraCreatedCallback);
        reg->events().subscribe(cameraDestroyedCallback);
    }

    slot_map_key renderable_manager::load_texture(const std::string& name, const texture_asset& asset)
    {
        auto width = asset.width();
        auto height = asset.height();
        auto channels = asset.channel_count();
        auto mipCount = asset.mip_count();
        auto swizzle = asset.swizzle();
        auto dataType = asset.type();
        auto bpp = 0u;

        data_format fmt = {};
        switch (dataType)
        {
        case texture_asset::data_type::UCHAR:
            if (channels != 4)
            {
                spdlog::error("Unknown texture asset format {} with {} channels.", as<std::uint32_t>(dataType), channels);
                return decltype(_textures)::invalid;
            }
            fmt = data_format::R8G8B8A8_SRGB;
            bpp = 4;
            break;
        case texture_asset::data_type::USHORT:
            if (channels != 4)
            {
                spdlog::error("Unknown texture asset format {} with {} channels.", as<std::uint32_t>(dataType), channels);
                return decltype(_textures)::invalid;
            }
            fmt = data_format::R16G16B16A16_SFLOAT;
            bpp = 8;
            break;
        default:
            spdlog::error("Unknown texture asset format {} with {} channels.", as<std::uint32_t>(dataType), channels);
            return decltype(_textures)::invalid;
        }

        const image_create_info cinfo = {
            .type = image_type::TYPE_2D,
            .format = fmt,
            .width = width,
            .height = height,
            .depth = 1,
            .mipLevels = mipCount,
            .arrayLayers = 1,
            .samples = sample_count::COUNT_1,
            .usage = image_usage::SAMPLED | image_usage::TRANSFER_DST
        };

        const allocation_create_info ainfo = {
            .required = memory_property::DEVICE_LOCAL,
            .preferred = {},
            .usage = memory_usage::PREFER_DEVICE,
            .hostSequentialWriteAccess = false,
            .hostRandomAccess = false,
            .persistentlyMapped = false
        };

        const auto imageResult = _manager->create(cinfo, ainfo);
        if (!imageResult)
        {
            spdlog::error("Failed to create image.");
            return decltype(_textures)::invalid;
        }

        const image_view_create_info vinfo = {
            .usage = std::nullopt,
            .img = *imageResult,
            .type = image_view_type::TYPE_2D,
            .fmt = cinfo.format,
            .subresource = {
                .aspect = image_aspect::COLOR,
                .baseMipLevel = 0,
                .mipLevelCount = mipCount,
                .baseLayer = 0,
                .layerCount = 1
            }
        };

        const auto imageViewResult = _manager->create(vinfo);
        if (!imageViewResult)
        {
            _manager->release(*imageResult);
            return decltype(_textures)::invalid;
        }

        auto cmdList = _manager->next_transfer_command_list();
        static_vector<buffer_image_copy_regions, 32> mipsToCopy;
        buffer activeBuffer = {};

        fence_create_info fenceInfo = {};
        fence waitFence = *_manager->create(fenceInfo);

        image_memory_barrier transitionFlags = {
            .src = access_type::NONE,
            .dst = access_type::TRANSFER_WRITE,
            .oldLayout = image_layout::UNDEFINED,
            .newLayout = image_layout::TRANSFER_DST_OPTIMAL,
            .srcQueue = cmdList.queue_index(),
            .dstQueue = cmdList.queue_index(),
            .img = *imageResult,
            .range = {
                .aspect = image_aspect::COLOR,
                .baseMipLevel = 0,
                .mipLevelCount = mipCount,
                .baseLayer = 0,
                .layerCount = 1,
            }
        };

        cmdList.begin();
        cmdList.barrier(pipeline_stage::TOP_OF_PIPE, pipeline_stage::TRANSFER, {}, {}, span(transitionFlags));
        cmdList.end();
        cmdList.submit({}, waitFence);
        _manager->wait(waitFence);
        _manager->reset(waitFence);

        cmdList = _manager->next_transfer_command_list();

        for (std::uint32_t i = 0; i < mipCount; ++i)
        {
            const auto& mip = asset.get_mip_level(i);
            auto imgWriteInfo = _manager->write_to_staging_buffer(mip->bytes.data(), mip->bytes.size());
            if (imgWriteInfo)
            {
                buffer_image_copy_regions region = {
                    .bufferOffset = 0,
                    .rowLength = 0,
                    .imageHeight = 0,
                    .subresource = {
                        .aspect = image_aspect::COLOR,
                        .mipLevel = i,
                        .baseArrayLayer = 0,
                        .layerCount = 1
                    },
                    .x = 0,
                    .y = 0,
                    .z = 0,
                    .width = mip->width,
                    .height = mip->height,
                    .depth = 1
                };

                mipsToCopy.push_back(region);

                if (activeBuffer.buffer != imgWriteInfo->buf.buffer && i != 0)
                {
                    cmdList.begin();
                    cmdList.copy(activeBuffer, *imageResult, image_layout::UNDEFINED, span(mipsToCopy.data(), mipsToCopy.size()));
                    cmdList.end();
                    cmdList.submit({}, waitFence);
                    _manager->wait(waitFence);
                    _manager->reset(waitFence);
                    mipsToCopy.clear();
                }
                activeBuffer = imgWriteInfo->buf;
            }
            else
            {
                cmdList.begin();
                cmdList.copy(activeBuffer, *imageResult, image_layout::UNDEFINED, span(mipsToCopy.data(), mipsToCopy.size()));
                cmdList.end();
                cmdList.submit({}, waitFence);
                _manager->wait(waitFence);
                _manager->reset(waitFence);
                mipsToCopy.clear();
                cmdList = _manager->next_transfer_command_list();

                _manager->reset_staging_buffer();
                imgWriteInfo = _manager->write_to_staging_buffer(mip->bytes.data(), mip->bytes.size());
                if (imgWriteInfo) {
                    buffer_image_copy_regions region = {
                        .bufferOffset = 0,
                        .rowLength = 0,
                        .imageHeight = 0,
                        .subresource = {
                            .aspect = image_aspect::COLOR,
                            .mipLevel = i,
                            .baseArrayLayer = 0,
                            .layerCount = 1
                        },
                        .x = 0,
                        .y = 0,
                        .z = 0,
                        .width = mip->width,
                        .height = mip->height,
                        .depth = 1
                    };

                    mipsToCopy.push_back(region);
                    activeBuffer = imgWriteInfo->buf;
                }
                else
                {
                    spdlog::warn("Staging buffer not sufficiently sized.");
                    // TODO: Allocate a temporary buffer to hold the image
                    // Write to the buffer, then destroy.
                }
            }
            
        }

        transitionFlags = {
            .src = access_type::TRANSFER_WRITE,
            .dst = access_type::MEMORY_READ,
            .oldLayout = image_layout::TRANSFER_DST_OPTIMAL,
            .newLayout = image_layout::SHADER_READ_ONLY_OPTIMAL,
            .srcQueue = cmdList.queue_index(),
            .dstQueue = cmdList.queue_index(),
            .img = *imageResult,
            .range = {
                .aspect = image_aspect::COLOR,
                .baseMipLevel = 0,
                .mipLevelCount = mipCount,
                .baseLayer = 0,
                .layerCount = 1,
            }
        };

        cmdList.begin();
        if (!mipsToCopy.empty())
        {
            cmdList.copy(activeBuffer, *imageResult, image_layout::TRANSFER_DST_OPTIMAL, span(mipsToCopy.data(), mipsToCopy.size()));
        }
        cmdList.barrier(pipeline_stage::TRANSFER, pipeline_stage::BOTTOM_OF_PIPE, {}, {}, {
            transitionFlags
        });
        cmdList.end();
        cmdList.submit({}, waitFence);
        _manager->wait(waitFence);
        _manager->reset(waitFence);
        mipsToCopy.clear();
        cmdList = _manager->next_transfer_command_list();

        const texture tex = {
            .img = *imageResult,
            .view = *imageViewResult,
            .sampler = _defaultSampler
        };

        _manager->release(waitFence);
        _manager->reset_staging_buffer();
        
        auto key = _textures.insert(tex);
        _textureNameLut[name] = key;

        return key;
    }

    slot_map_key renderable_manager::load_texture(const std::string& name, const image img, const image_view view)
    {
        const texture tex = {
            .img = img,
            .view = view,
            .sampler = _defaultSampler
        };
        auto key = _textures.insert(tex);
        _textureNameLut[name] = key;
        return key;
    }

    std::optional<texture> renderable_manager::try_fetch_texture(const std::string& name)
    {
        auto it = _textureNameLut.find(name);
        if (it != _textureNameLut.end())
        {
            return try_fetch_texture(it->second);
        }
        return std::nullopt;
    }

    std::optional<texture> renderable_manager::try_fetch_texture(const slot_map_key& key)
    {
        auto tex = _textures.try_get(key);
        if (tex)
        {
            return *tex;
        }
        return std::nullopt;
    }

    void renderable_manager::unload_texture(const slot_map_key& key)
    {
        auto tex = try_fetch_texture(key);
        if (tex)
        {
            _textures.erase(key);
            _manager->release(tex->img);
            _manager->release(tex->view);
        }
    }

    slot_map_key renderable_manager::load_material(const std::string& name, const material_asset& asset)
    {
        const auto base = asset.baseColorTexture;
        const auto normal = asset.normalTexture;
        const auto metalRough = asset.metallicRoughness;
        const auto emissive = asset.emissiveTexture;
        const auto occlusion = asset.occlusionTexture;

        const auto baseTex = base ? load_texture(fmt::v8::format("{}_{}", name, "base"), *base) : decltype(_textures)::invalid;
        const auto normalTex = normal ? load_texture(fmt::v8::format("{}_{}", name, "normal"), *normal) : decltype(_textures)::invalid;
        const auto metalRoughTex = metalRough ? load_texture(fmt::v8::format("{}_{}", name, "metalRough"), *metalRough) : decltype(_textures)::invalid;
        const auto emissiveTex = emissive ? load_texture(fmt::v8::format("{}_{}", name, "emissive"), *emissive) : decltype(_textures)::invalid;
        const auto occlusionTex = occlusion ? load_texture(fmt::v8::format("{}_{}", name, "occlusion"), *occlusion) : decltype(_textures)::invalid;

        const material mat = {
            .albedo = baseTex,
            .normal = normalTex,
            .metallicRoughness = metalRoughTex,
            .emissive = emissiveTex,
            .ao = occlusionTex
        };

        return _materials.insert(mat);
    }

    slot_map_key renderable_manager::load_mesh(const std::string& name, const mesh& m)
    {
        renderable_mesh mesh
        {
            .bufferGroupId = as<std::uint32_t>(_bakedBufferGroups.size()),
            .vertexOffset = as<std::uint32_t>(_activeMeshGroup.positions.size()),
            .indexOffset = as<std::uint32_t>(_activeMeshGroup.indices.size()),
            .indexCount = as<std::uint32_t>(m.indices.size())
        };

        auto requestedVerts = _activeMeshGroup.positions.size() + m.vertices.size();
        auto requestedIndices = _activeMeshGroup.indices.size() + m.indices.size();
        _activeMeshGroup.positions.reserve(requestedVerts);
        _activeMeshGroup.interleavedValues.reserve(requestedVerts);
        _activeMeshGroup.indices.reserve(requestedIndices);

        for (const auto& vertex : m.vertices)
        {
            mesh_group::position_t positionAttrib = {
                .x = vertex.position.x,
                .y = vertex.position.y,
                .z = vertex.position.z
            };

            mesh_group::interleaved_t interleavedAttribs = {
                .texcoord0 = {
                    .u = vertex.texCoord.u,
                    .v = vertex.texCoord.v
                },
                .normal = {
                    .x = vertex.normal.x,
                    .y = vertex.normal.y,
                    .z = vertex.normal.z
                },
                .tangent = {
                    .x = vertex.tangent.x,
                    .y = vertex.tangent.y,
                    .z = vertex.tangent.z,
                    .w = vertex.tangent.w
                }
            };

            _activeMeshGroup.positions.push_back(positionAttrib);
            _activeMeshGroup.interleavedValues.push_back(interleavedAttribs);
        }

        for (const auto index : m.indices)
        {
            _activeMeshGroup.indices.push_back(index);
        }

        return _meshes.insert(mesh);
    }

    void renderable_manager::build_meshes()
    {
        // build staging buffer
        const std::size_t positionsSize = sizeof(mesh_group::position_t) * _activeMeshGroup.positions.size();
        const std::size_t interleavedSize = sizeof(mesh_group::interleaved_t) * _activeMeshGroup.interleavedValues.size();
        const std::size_t indicesSize = sizeof(std::uint32_t) * _activeMeshGroup.indices.size();
        const buffer_create_info stagingInfo = {
            .size = (positionsSize + interleavedSize + indicesSize),
            .usage = buffer_usage::TRANSFER_SRC
        };

        const allocation_create_info stagingAllocInfo = {
            .required = memory_property::HOST_COHERENT | memory_property::HOST_VISIBLE,
            .preferred = as<memory_property>(0),
            .usage = memory_usage::PREFER_HOST,
            .hostSequentialWriteAccess = false,
            .hostRandomAccess = true,
            .persistentlyMapped = true
        };

        const auto stagingResult = _manager->create(stagingInfo, stagingAllocInfo);
        if (!stagingResult)
        {
            spdlog::error("Failed to create staging buffer of size {}.", stagingInfo.size);
            return;
        }

        const auto stagingBuffer = *stagingResult;
        std::byte* gpuBufferAddr = reinterpret_cast<std::byte*>(stagingBuffer.info.pMappedData);
#ifdef _RYUJIN_WINDOWS
        memcpy_s(gpuBufferAddr, positionsSize, _activeMeshGroup.positions.data(), positionsSize);
        memcpy_s(gpuBufferAddr + positionsSize, interleavedSize, _activeMeshGroup.interleavedValues.data(), interleavedSize);
        memcpy_s(gpuBufferAddr + positionsSize + interleavedSize, indicesSize, _activeMeshGroup.indices.data(), indicesSize);
#else
        memcpy(gpuBufferAddr, _activeMeshGroup.positions.data(), positionsSize);
        memcpy(gpuBufferAddr + positionsSize, _activeMeshGroup.interleavedValues.data(), interleavedSize);
        memcpy(gpuBufferAddr + positionsSize + interleavedSize, _activeMeshGroup.indices.data(), indicesSize);
#endif

        // build vertex and index buffers
        const buffer_create_info positionBufferCreateInfo = {
            .size = positionsSize,
            .usage = buffer_usage::TRANSFER_DST | buffer_usage::VERTEX
        };

        const buffer_create_info interleavedBufferCreateInfo = {
            .size = interleavedSize,
            .usage = buffer_usage::TRANSFER_DST | buffer_usage::VERTEX
        };

        const buffer_create_info indexBufferCreateInfo = {
            .size = interleavedSize,
            .usage = buffer_usage::TRANSFER_DST | buffer_usage::INDEX
        };

        const allocation_create_info gpuAllocInfo = {
            .required = memory_property::DEVICE_LOCAL,
            .preferred = as<memory_property>(0),
            .usage = memory_usage::PREFER_DEVICE,
            .hostSequentialWriteAccess = false,
            .hostRandomAccess = false,
            .persistentlyMapped = false
        };

        const auto positionsBufferResult = _manager->create(positionBufferCreateInfo, gpuAllocInfo);
        if (!positionsBufferResult)
        {
            spdlog::error("Failed to create vertex buffer for positions.");
            return;
        }

        const auto interleavedBufferResult = _manager->create(interleavedBufferCreateInfo, gpuAllocInfo);
        if (!interleavedBufferResult)
        {
            spdlog::error("Failed to create vertex buffer for interleaved attributes.");
            _manager->release(*positionsBufferResult);
            return;
        }

        const auto indexBufferResult = _manager->create(indexBufferCreateInfo, gpuAllocInfo);
        if (!indexBufferResult)
        {
            spdlog::error("Failed to create index buffer.");
            _manager->release(*interleavedBufferResult);
            _manager->release(*positionsBufferResult);
            return;
        }

        auto& positionsBuffer = *positionsBufferResult;
        auto& interleavedBuffer = *interleavedBufferResult;
        auto& indexBuffer = *indexBufferResult;

        buffer_copy_regions positionRegion = {
            .srcOffset = 0,
            .dstOffset = 0,
            .size = positionsSize
        };

        buffer_copy_regions interleavedRegion = {
            .srcOffset = positionsSize,
            .dstOffset = 0,
            .size = interleavedSize
        };

        buffer_copy_regions indexRegion = {
            .srcOffset = positionsSize + interleavedSize,
            .dstOffset = 0,
            .size = indicesSize
        };

        auto transfer = _manager->next_transfer_command_list();
        transfer.begin();
        transfer.copy(stagingBuffer, positionsBuffer, span(positionRegion));
        transfer.copy(stagingBuffer, interleavedBuffer, span(interleavedRegion));
        transfer.copy(stagingBuffer, indexBuffer, span(indexRegion));
        transfer.end();
        
        const fence_create_info syncInfo = {
            .signaled = false
        };

        const auto syncFence = *_manager->create(syncInfo);

        transfer.submit({
            .wait = {},
            .signal = {}
        }, syncFence);

        _manager->wait(syncFence);
        _manager->release(stagingBuffer, true);
        _manager->release(syncFence, true);

        spdlog::info("Mesh building successful. Built positions buffer ({}), interleaved components buffer ({}), and indices buffer ({}).", _activeMeshGroup.positions.size(), _activeMeshGroup.interleavedValues.size(), _activeMeshGroup.indices.size());
        spdlog::info("Clearing active mesh build group.");

        const buffer_group bufGroup = {
            .positions = positionsBuffer,
            .interleaved = interleavedBuffer,
            .indices = indexBuffer
        };
        
        _bakedBufferGroups.push_back(bufGroup);

        _activeMeshGroup.clear();
    }
    
    std::size_t renderable_manager::write_instances(buffer& buf, const std::size_t offset)
    {
        std::size_t instance = 0;
        auto instances = reinterpret_cast<gpu_instance_data*>(buf.info.pMappedData) + offset;
        for (const auto& [meshGroupId, meshMapping] : _entities)
        {
            for (const auto& [key, ents] : meshMapping)
            {
                for (const auto& ent : ents)
                {
                    entity_handle handle(ent, _registry);
                    auto tx = handle.get<transform_component>();
                    auto matHandle = handle.get<renderable_component>().material;
                    auto mat = _materials.index_of(matHandle);
                    instances[instance].transform = tx.matrix;
                    instances[instance].material = as<std::uint32_t>(mat);
                    ++instance;
                }
            }
        }
        return instance;
    }

    std::size_t renderable_manager::write_materials(buffer& buf, const std::size_t offset)
    {
        // TODO: figure out some way to do this only when a new material is added or a 
        std::size_t materialId = 0;
        auto materials = reinterpret_cast<gpu_material_data*>(buf.info.pMappedData) + offset;

        for (const auto& material : _materials)
        {
            const auto albedo = material.albedo;
            const auto normal = material.normal;
            const auto metalRough = material.metallicRoughness;
            const auto emissive = material.emissive;
            const auto ao = material.ao;

            const gpu_material_data data = {
                .albedo = as<std::uint32_t>(_textures.index_of(albedo)),
                .normal = as<std::uint32_t>(_textures.index_of(normal)),
                .metallicRoughness = as<std::uint32_t>(_textures.index_of(metalRough)),
                .emissive = as<std::uint32_t>(_textures.index_of(emissive)),
                .ambientOcclusion = as<std::uint32_t>(_textures.index_of(ao))
            };

            materials[materialId++] = data;
        }
        return materialId;
    }

    std::size_t renderable_manager::write_draw_calls(buffer& indirectBuffer, buffer& drawCountBuffer, const std::size_t offset)
    {
        // TODO: figure out some way to only do this when we change the mapping
        std::uint32_t firstInstance = 0;
        std::uint32_t meshGroupsWritten = 0;

        for (const auto& [meshGroupId, meshMapping] : _entities)
        {
            std::uint32_t drawCallCount = 0;

            for (const auto& [key, ents] : meshMapping)
            {
                auto mesh = _meshes.try_get(key);
                gpu_indirect_call call = {
                    .indexCount = mesh->indexCount,
                    .instanceCount = as<std::uint32_t>(ents.size()),
                    .firstIndex = mesh->indexOffset,
                    .vertexOffset = as<std::int32_t>(mesh->vertexOffset),
                    .firstInstance = firstInstance
                };

                auto mapped = reinterpret_cast<gpu_indirect_call*>(indirectBuffer.info.pMappedData) + offset + drawCallCount;
                *mapped = call;

                firstInstance += as<std::uint32_t>(ents.size());
                ++drawCallCount;
            }
            auto mapped = reinterpret_cast<std::uint32_t*>(drawCountBuffer.info.pMappedData) + offset + meshGroupsWritten;
            *mapped = as<std::uint32_t>(meshMapping.size());
            ++meshGroupsWritten;
        }
        spdlog::trace("Writing {} mesh groups to indirect commands.", meshGroupsWritten);
        return meshGroupsWritten;
    }

    std::size_t renderable_manager::write_textures(texture* buf, std::size_t offset)
    {
        std::size_t textureId = 0;
        for (auto& tex : _textures)
        {
            buf[textureId++] = tex;
        }
        return textureId;
    }

    const renderable_manager::buffer_group& renderable_manager::get_buffer_group(const std::size_t idx) const noexcept
    {
        return _bakedBufferGroups[idx];
    }

    entity_handle<registry::entity_type> renderable_manager::main_camera() noexcept
    {
        auto cameraEntityView = _registry->entity_view<camera_component>();
        for (const auto& e : cameraEntityView)
        {
            return e;
        }
        return _registry->invalid();
    }

    void renderable_manager::get_active_cameras(vector<entity_handle<entity_type>>& entities)
    {
        for (const auto& [order, handles] : _cameras)
        {
            for (auto& handle : handles)
            {
                entity_handle e(handle, _registry);
                auto& cam = e.get<camera_component>();
                if (cam.active)
                {
                    entities.push_back(e);
                }
            }
        }
    }

    void renderable_manager::register_entity(entity_type ent)
    {
        entity_handle e(ent, _registry);
        auto renderable = e.try_get<renderable_component>();
        if (renderable)
        {
            auto mesh = renderable->mesh;
            auto group = _meshes.try_get(mesh);
            _entities[group->bufferGroupId][mesh].push_back(ent);
        }
    }

    void renderable_manager::unregister_entity(entity_type ent)
    {
        entity_handle e(ent, _registry);
        auto renderable = e.try_get<renderable_component>();
        if (renderable)
        {
            auto mesh = renderable->mesh;
            auto group = _meshes.try_get(mesh);
            auto& ents = _entities[group->bufferGroupId][mesh];
            auto it = std::find(ents.begin(), ents.end(), ent);
            if (it != ents.end())
            {
                ents.erase(it);
            }
            else
            {
                spdlog::warn("Failed to remove previous entity from mesh list. Not sure how we got here.");
            }
        }
    }

    void renderable_manager::update_entity(entity_type ent)
    {
        unregister_entity(ent);
        register_entity(ent);
    }

    void renderable_manager::register_camera(entity_type ent)
    {
        entity_handle e(ent, _registry);
        auto camera = e.try_get<camera_component>();
        if (camera)
        {
            _cameras[camera->order].push_back(ent);
        }
    }

    void renderable_manager::unregister_camera(entity_type ent)
    {
        entity_handle e(ent, _registry);
        auto camera = e.try_get<camera_component>();
        if (camera)
        {
            auto& cams = _cameras[camera->order];
            auto it = std::find(cams.begin(), cams.end(), ent);
            if (it != cams.end())
            {
                cams.erase(it);
            }
        }
    }

    void renderable_manager::mesh_group::clear()
    {
        positions.clear();
        interleavedValues.clear();
        indices.clear();
    }

}