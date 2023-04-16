#include <graphics/descriptor.h>
#include <graphics/asset_pool.h>
#include <graphics/graphics_context.h>
#include <graphics/resource/buffer.h>
#include <graphics/resource/image.h>

namespace Sunset
{
	Sunset::DescriptorSetBuilder DescriptorSetBuilder::begin(class GraphicsContext* const context)
	{
		DescriptorSetBuilder builder;
		builder.gfx_context = context;
		return builder;
	}

	Sunset::DescriptorSetBuilder& DescriptorSetBuilder::bind_buffer(uint16_t binding, Buffer* buffer, size_t range_size, uint32_t count, DescriptorType type, PipelineShaderStageType shader_stages, size_t buffer_offset, bool b_supports_bindless)
	{
		bindings.push_back({ .slot = binding, .count = count, .type = type, .pipeline_stages = shader_stages, .b_supports_bindless = b_supports_bindless });
		if (buffer != nullptr)
		{
			DescriptorBufferDesc buffer_desc{ .buffer = buffer->get(), .buffer_size = buffer->get_size(), .buffer_range = range_size, .buffer_offset = buffer_offset };
			descriptor_writes.push_back({ .slot = binding, .count = count, .type = type, .buffer_desc = buffer_desc, .set = nullptr });
		}
		return *this;
	}

	Sunset::DescriptorSetBuilder& DescriptorSetBuilder::bind_buffer(const DescriptorBuildData& build_data)
	{
		Buffer* const buffer = CACHE_FETCH(Buffer, build_data.buffer);
		return bind_buffer(build_data.binding, buffer, build_data.buffer_range, build_data.count, build_data.type, build_data.shader_stages, build_data.buffer_offset, build_data.b_supports_bindless);
	}

	Sunset::DescriptorSetBuilder& DescriptorSetBuilder::bind_image(uint16_t binding, Image* image, size_t range_size, uint32_t count, DescriptorType type, PipelineShaderStageType shader_stages, bool b_supports_bindless)
	{
		bindings.push_back({ .slot = binding, .count = count, .type = type, .pipeline_stages = shader_stages, .b_supports_bindless = b_supports_bindless });
		if (image != nullptr)
		{
			DescriptorBufferDesc buffer_desc{ .buffer = image, .buffer_size = 0, .buffer_range = range_size, .buffer_offset = 0 };
			descriptor_writes.push_back({ .slot = binding, .count = count, .type = type, .buffer_desc = buffer_desc, .set = nullptr });
		}
		return *this;
	}

	Sunset::DescriptorSetBuilder& DescriptorSetBuilder::bind_image(const DescriptorBuildData& build_data)
	{
		Image* const image = CACHE_FETCH(Image, build_data.image);
		return bind_image(build_data.binding, image, build_data.buffer_range, build_data.count, build_data.type, build_data.shader_stages, build_data.b_supports_bindless);
	}

	bool DescriptorSetBuilder::build(DescriptorSet*& out_descriptor_set, DescriptorLayoutID& out_descriptor_layout)
	{
		if (out_descriptor_layout == 0)
		{
			out_descriptor_layout = DescriptorLayoutFactory::create(gfx_context, bindings);
		}
		if (out_descriptor_set == nullptr)
		{
			out_descriptor_set = gfx_context->get_descriptor_set_allocator()->allocate(gfx_context, CACHE_FETCH(DescriptorLayout, out_descriptor_layout));
			if (out_descriptor_set == nullptr)
			{
				return false;
			}
			for (const DescriptorBinding& binding : bindings)
			{
				if (binding.b_supports_bindless)
				{
					out_descriptor_set->register_bindless_slot(binding.slot, binding.count);
				}
			}
		}

		DescriptorHelpers::write_descriptors(gfx_context, out_descriptor_set, descriptor_writes);

		return true;
	}

	void DescriptorHelpers::inject_descriptors(GraphicsContext* const gfx_context, DescriptorData& out_descriptor_data, const std::vector<DescriptorBuildData>& descriptor_build_datas)
	{
		DescriptorSetBuilder builder = DescriptorSetBuilder::begin(gfx_context);
		for (const DescriptorBuildData& descriptor_build_data : descriptor_build_datas)
		{
			if (descriptor_build_data.type == DescriptorType::Image)
			{
				builder.bind_image(descriptor_build_data);
			}
			else
			{
				builder.bind_buffer(descriptor_build_data);
			}
		}
		builder.build(out_descriptor_data.descriptor_set, out_descriptor_data.descriptor_layout);
	}

	DescriptorSet* DescriptorHelpers::new_descriptor_set_with_layout(GraphicsContext* const gfx_context, DescriptorLayoutID descriptor_layout)
	{
		assert(descriptor_layout != 0);

		DescriptorLayout* const descriptor_layout_obj = CACHE_FETCH(DescriptorLayout, descriptor_layout);
		DescriptorSet* descriptor_set = gfx_context->get_descriptor_set_allocator()->allocate(gfx_context, descriptor_layout_obj);

		if (descriptor_set == nullptr)
		{
			return nullptr;
		}

		std::vector<DescriptorBinding> bindings = descriptor_layout_obj->get_bindings();
		for (const DescriptorBinding& binding : bindings)
		{
			if (binding.b_supports_bindless)
			{
				descriptor_set->register_bindless_slot(binding.slot, binding.count);
			}
		}

		return descriptor_set;
	}

	void DescriptorHelpers::write_descriptors(GraphicsContext* const gfx_context, DescriptorSet* descriptor_set, std::vector<DescriptorWrite>& descriptor_writes)
	{
		if (!descriptor_writes.empty())
		{
			// Set our dynamic uniform/SSBO offsets if we have any
			{
				std::vector<uint32_t> dynamic_buffer_offsets;
				for (DescriptorWrite& write : descriptor_writes)
				{
					write.set = descriptor_set;

					if (write.type == DescriptorType::DynamicUniformBuffer)
					{
						dynamic_buffer_offsets.push_back(write.buffer_desc.buffer_offset);
					}
				}
				descriptor_set->set_dynamic_buffer_offsets(dynamic_buffer_offsets);
			}

			// Push our descriptors through the GPI
			gfx_context->push_descriptor_writes(descriptor_writes);
		}
	}

	void DescriptorHelpers::write_bindless_descriptors(class GraphicsContext* const gfx_context, const std::vector<DescriptorBindlessWrite>& descriptor_writes, BindingTableHandle* out_binding_table_handles)
	{
		std::vector<DescriptorWrite> writes;

		for (uint32_t i = 0; i < descriptor_writes.size(); ++i)
		{
			const DescriptorBindlessWrite& bindless_write = descriptor_writes[i];

			if (out_binding_table_handles[i] < 0)
			{
				out_binding_table_handles[i] = bindless_write.set->get_free_bindless_index(bindless_write.slot);
			}

			DescriptorWrite& write = writes.emplace_back();
			write.slot = bindless_write.slot;
			write.count = 1;
			write.array_index = 0x0000ffff & out_binding_table_handles[i];
			write.type = bindless_write.type;
			write.buffer_desc.buffer = bindless_write.buffer;
			write.buffer_desc.buffer_offset = bindless_write.level;
			write.set = bindless_write.set;
		}

		gfx_context->push_descriptor_writes(writes);
	}

	void DescriptorHelpers::free_bindless_image_descriptors(class GraphicsContext* const gfx_context, DescriptorSet* descriptor_set, const std::vector<BindingTableHandle> handles)
	{
		for (BindingTableHandle handle : handles)
		{
			descriptor_set->release_bindless_index(handle);
		}
	}

	std::vector<DescriptorBindlessWrite> DescriptorHelpers::new_descriptor_image_bindless_writes(class DescriptorSet* set, ImageID image, bool b_split_image_mips)
	{
		std::vector<DescriptorBindlessWrite> writes;

		Image* const image_obj = CACHE_FETCH(Image, image);

		if (b_split_image_mips)
		{
			const uint32_t num_mips = image_obj->get_attachment_config().mip_count;
			writes.reserve(num_mips);
			for (uint32_t i = 0; i < num_mips; ++i)
			{
				writes.push_back(
					{
						.slot = ImageBindTableSlot,
						.level = i,
						.type = DescriptorType::Image,
						.buffer = CACHE_FETCH(Image, image),
						.set = set
					}
				);
			}
			for (uint32_t i = 0; i < num_mips; ++i)
			{
				if ((image_obj->get_attachment_config().flags & ImageFlags::Storage) != ImageFlags::None)
				{
					writes.push_back(
						{
							.slot = StorageImageBindTableSlot,
							.level = i,
							.type = DescriptorType::StorageImage,
							.buffer = CACHE_FETCH(Image, image),
							.set = set
						}
					);
				}
			}
		}
		else
		{
			writes.push_back(
				{
					.slot = ImageBindTableSlot,
					.type = DescriptorType::Image,
					.buffer = CACHE_FETCH(Image, image),
					.set = set
				}
			);
			if ((image_obj->get_attachment_config().flags & ImageFlags::Storage) != ImageFlags::None)
			{
				writes.push_back(
					{
						.slot = StorageImageBindTableSlot,
						.type = DescriptorType::StorageImage,
						.buffer = CACHE_FETCH(Image, image),
						.set = set
					}
				);
			}
		}

		return writes;
	}

	bool DescriptorBindingTable::has_binding_slot(uint32_t slot)
	{
		return binding_table.contains(slot);
	}

	void DescriptorBindingTable::add_binding_slot(uint32_t slot, uint32_t count)
	{
		binding_table.insert({ slot, {} });
		binding_table[slot].total_bindings_count = count;
		reset(slot);
	}

	BindingTableHandle DescriptorBindingTable::get_new(uint32_t slot)
	{
		assert(binding_table.contains(slot));

		if (binding_table[slot].free_indices.empty())
		{
			return -1;
		}

		int32_t new_index = binding_table[slot].free_indices.back();
		binding_table[slot].free_indices.pop_back();
		binding_table[slot].bound_indices.push_back(new_index);

		BindingTableHandle handle = (((int64_t)slot) << 32) | (int64_t)new_index;

		return handle;
	}

	void DescriptorBindingTable::free(BindingTableHandle handle)
	{
		uint32_t slot = 0x0000ffff & (handle >> 32);
		uint32_t index = 0x0000ffff & handle;

		assert(binding_table.contains(slot));

		// TODO: Consider keeping fetched handles in an unordered_set so we don't have to do a costly linear search for existing slot indices,
		// but only if index allocations at any given time start becoming numerous
		if (std::find(binding_table[slot].bound_indices.begin(), binding_table[slot].bound_indices.end(), index) != binding_table[slot].bound_indices.end())
		{
			binding_table[slot].free_indices.push_back(index);
			binding_table[slot].bound_indices.erase(
				std::remove(binding_table[slot].bound_indices.begin(), binding_table[slot].bound_indices.end(), index),
				binding_table[slot].bound_indices.end()
			);
		}
	}

	void DescriptorBindingTable::reset(uint32_t slot)
	{
		assert(binding_table.contains(slot));

		binding_table[slot].free_indices.clear();
		binding_table[slot].bound_indices.clear();

		binding_table[slot].free_indices.reserve(binding_table[slot].total_bindings_count);
		for (int32_t i = binding_table[slot].total_bindings_count - 1; i >= 0; --i)
		{
			binding_table[slot].free_indices.push_back(i);
		}
	}

	Sunset::DescriptorLayoutID DescriptorLayoutFactory::create(class GraphicsContext* const gfx_context, const std::vector<DescriptorBinding>& bindings)
	{
		Identity cache_id;
		for (const DescriptorBinding& binding : bindings)
		{
			cache_id = Maths::cantor_pair_hash(static_cast<int32_t>(cache_id), static_cast<int32_t>(std::hash<DescriptorBinding>{}(binding)));
		}

		bool b_added{ false };
		DescriptorLayoutID layout_id = DescriptorLayoutCache::get()->fetch_or_add(cache_id, gfx_context, b_added);
		if (b_added)
		{
			DescriptorLayout* const descriptor_layout = CACHE_FETCH(DescriptorLayout, layout_id);
			descriptor_layout->set_bindings(bindings);
			descriptor_layout->build(gfx_context);
		}

		return layout_id;
	}
}
