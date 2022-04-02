#ifndef type_converter_hpp__
#define type_converter_hpp__

#include <ryujin/core/as.hpp>
#include <ryujin/graphics/types.hpp>

#include <vulkan/vulkan.h>

namespace ryujin
{
	inline constexpr VkFormat to_vulkan(const data_format fmt)
	{
		return as<VkFormat>(fmt);
	}

	inline constexpr VkSampleCountFlagBits to_vulkan(const sample_count samples)
	{
		return as<VkSampleCountFlagBits>(samples);
	}

	inline constexpr VkAttachmentLoadOp to_vulkan(const attachment_load_op op)
	{
		return as<VkAttachmentLoadOp>(op);
	}

	inline constexpr VkAttachmentStoreOp to_vulkan(const attachment_store_op op)
	{
		return as<VkAttachmentStoreOp>(op);
	}

	inline constexpr VkImageLayout to_vulkan(const image_layout layout)
	{
		return as<VkImageLayout>(layout);
	}

	inline constexpr VkBufferUsageFlags to_vulkan(const buffer_usage usage)
	{
		return as<VkBufferUsageFlags>(usage);
	}

	inline constexpr VkImageUsageFlags to_vulkan(const image_usage usage)
	{
		return as<VkImageUsageFlags>(usage);
	}

	inline constexpr VkImageAspectFlags to_vulkan(const image_aspect aspect)
	{
		return as<VkImageAspectFlags>(aspect);
	}

	inline constexpr VkPipelineStageFlags to_vulkan(const pipeline_stage stage)
	{
		return as<VkPipelineStageFlags>(stage);
	}

	inline constexpr VkAccessFlags to_vulkan(const access_type access)
	{
		return as<VkAccessFlags>(access);
	}

	inline constexpr VkShaderStageFlagBits to_vulkan(const shader_stage stage)
	{
		return as<VkShaderStageFlagBits>(stage);
	}

	inline constexpr VkVertexInputRate to_vulkan(const input_binding_rate rate)
	{
		return as<VkVertexInputRate>(rate);
	}

	inline constexpr VkPrimitiveTopology to_vulkan(const primitive_topology topo)
	{
		return as<VkPrimitiveTopology>(topo);
	}

	inline constexpr VkCompareOp to_vulkan(const compare_op op)
	{
		return as<VkCompareOp>(op);
	}

	inline constexpr VkPolygonMode to_vulkan(const polygon_rasterization_mode mode)
	{
		return as<VkPolygonMode>(mode);
	}

	inline constexpr VkCullModeFlags to_vulkan(const cull_mode mode)
	{
		return as<VkCullModeFlags>(mode);
	}

	inline constexpr VkFrontFace to_vulkan(const vertex_winding_order face)
	{
		return as<VkFrontFace>(face);
	}

	inline constexpr VkStencilOp to_vulkan(const stencil_op face)
	{
		return as<VkStencilOp>(face);
	}

	inline constexpr VkLogicOp to_vulkan(const logic_op face)
	{
		return as<VkLogicOp>(face);
	}

	inline constexpr VkBlendFactor to_vulkan(const blend_factor factor)
	{
		return as<VkBlendFactor>(factor);
	}

	inline constexpr VkBlendOp to_vulkan(const blend_op op)
	{
		return as<VkBlendOp>(op);
	}

	inline constexpr VkDynamicState to_vulkan(const dynamic_pipeline_state state)
	{
		return as<VkDynamicState>(state);
	}

	inline constexpr VkImageType to_vulkan(const image_type type)
	{
		return as<VkImageType>(type);
	}

	inline constexpr VkImageViewType to_vulkan(const image_view_type type)
	{
		return as<VkImageViewType>(type);
	}

	inline constexpr VkDescriptorType to_vulkan(const descriptor_type type)
	{
		return as<VkDescriptorType>(type);
	}

	inline constexpr VmaMemoryUsage to_vma(const memory_usage usage)
	{
		return as<VmaMemoryUsage>(usage);
	}

	inline constexpr VkMemoryPropertyFlags to_vulkan(const memory_property props)
	{
		return as<VkMemoryPropertyFlags>(props);
	}

	inline constexpr VkFilter to_vulkan(const filter fil)
	{
		return as<VkFilter>(fil);
	}

	inline constexpr VkSamplerMipmapMode to_vulkan(const mipmap_mode mode)
	{
		return as<VkSamplerMipmapMode>(mode);
	}

	inline constexpr VkSamplerAddressMode to_vulkan(const address_mode mode)
	{
		return as<VkSamplerAddressMode>(mode);
	}
}

#endif // type_converter_hpp__
