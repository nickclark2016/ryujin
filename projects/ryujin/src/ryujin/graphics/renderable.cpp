#include <ryujin/graphics/renderable.hpp>

#include <ryujin/core/as.hpp>
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
            fmt = data_format::R8G8B8A8_UINT;
            bpp = 4;
            break;
        case texture_asset::data_type::USHORT:
            if (channels != 4)
            {
                spdlog::error("Unknown texture asset format {} with {} channels.", as<std::uint32_t>(dataType), channels);
                return decltype(_textures)::invalid;
            }
            fmt = data_format::R16G16B16A16_UINT;
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

    renderable_mesh renderable_manager::load_mesh(const std::string& name, const mesh& m)
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

        return mesh;
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
    }

    void renderable_manager::register_entity(entity_type ent)
    {
        entity_handle e(ent, _registry);
        auto renderable = e.try_get<renderable_component>();
        if (renderable)
        {
            auto mesh = renderable->mesh;
            _entities[mesh].push_back(ent);
        }
    }
}