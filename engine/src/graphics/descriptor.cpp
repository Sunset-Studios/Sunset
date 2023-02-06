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

	Sunset::DescriptorSetBuilder& DescriptorSetBuilder::bind_buffer(uint16_t binding, Buffer* buffer, size_t range_size, uint32_t count, DescriptorType type, PipelineShaderStageType shader_stages, bool b_supports_bindless)
	{
		bindings.push_back({ .slot = binding, .count = count, .type = type, .pipeline_stages = shader_stages, .b_supports_bindless = b_supports_bindless });
		if (buffer != nullptr)
		{
			descriptor_writes.push_back({ .slot = binding, .count = count, .type = type, .buffer = buffer->get(), .buffer_size = buffer->get_size(), .buffer_range = range_size, .set = nullptr });
		}
		return *this;
	}

	Sunset::DescriptorSetBuilder& DescriptorSetBuilder::bind_buffer(const DescriptorBuildData& build_data)
	{
		Buffer* const buffer = CACHE_FETCH(Buffer, build_data.buffer);
		return bind_buffer(build_data.binding, buffer, build_data.buffer_range, build_data.count, build_data.type, build_data.shader_stages, build_data.b_supports_bindless);
	}

	Sunset::DescriptorSetBuilder& DescriptorSetBuilder::bind_image(uint16_t binding, Image* image, size_t range_size, uint32_t count, DescriptorType type, PipelineShaderStageType shader_stages, bool b_supports_bindless)
	{
		bindings.push_back({ .slot = binding, .count = count, .type = type, .pipeline_stages = shader_stages, .b_supports_bindless = b_supports_bindless });
		if (image != nullptr)
		{
			descriptor_writes.push_back({ .slot = binding, .count = count, .type = type, .buffer = image, .buffer_size = 0, .buffer_range = range_size, .set = nullptr });
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
		Identity cache_id;
		for (const DescriptorBinding& binding : bindings)
		{
			cache_id = Maths::cantor_pair_hash(static_cast<int32_t>(cache_id), static_cast<int32_t>(std::hash<DescriptorBinding>{}(binding)));
		}

		bool b_layout_changed = false;
		if (!CACHE_EXISTS(DescriptorLayout, cache_id))
		{
			out_descriptor_layout = DescriptorLayoutFactory::create(gfx_context, bindings);
			b_layout_changed = true;
		}
		
		if (b_layout_changed)
		{
			out_descriptor_set = gfx_context->get_descriptor_set_allocator()->allocate(gfx_context, CACHE_FETCH(DescriptorLayout, out_descriptor_layout));
			if (out_descriptor_set == nullptr)
			{
				return false;
			}
		}

		for (DescriptorWrite& write : descriptor_writes)
		{
			write.set = out_descriptor_set;
		}

		for (const DescriptorBinding& binding : bindings)
		{
			if (binding.b_supports_bindless)
			{
				out_descriptor_set->register_bindless_slot(binding.slot, binding.count);
			}
		}

		if (!descriptor_writes.empty())
		{
			gfx_context->push_descriptor_writes(descriptor_writes);
		}

		return true;
	}

	void DescriptorHelpers::inject_descriptors(GraphicsContext* const context, DescriptorData& out_descriptor_data, const std::vector<DescriptorBuildData>& descriptor_build_datas)
	{
		if (out_descriptor_data.descriptor_set == nullptr)
		{
			DescriptorSetBuilder builder = DescriptorSetBuilder::begin(context);
			for (const DescriptorBuildData& descriptor_build_data : descriptor_build_datas)
			{
				// TODO: Switch on descriptor type to determine whether to bind buffer or image
				if (descriptor_build_data.type == DescriptorType::Image)
				{
					builder.bind_image(descriptor_build_data);
				}
				else
				{
					builder.bind_buffer(descriptor_build_data);
				}

				if (descriptor_build_data.type == DescriptorType::DynamicUniformBuffer)
				{
					out_descriptor_data.dynamic_buffer_offsets.push_back(descriptor_build_data.buffer_offset);
				}
			}
			builder.build(out_descriptor_data.descriptor_set, out_descriptor_data.descriptor_layout);
		}
	}

	void DescriptorHelpers::write_bindless_descriptors(class GraphicsContext* const context, const std::vector<DescriptorBindlessWrite>& descriptor_writes, int32_t* out_array_indices)
	{
		std::vector<DescriptorWrite> writes;

		for (int i = 0; i < descriptor_writes.size(); ++i)
		{
			const DescriptorBindlessWrite& bindless_write = descriptor_writes[i];

			out_array_indices[i] = bindless_write.set->get_free_bindless_index(bindless_write.slot);

			DescriptorWrite& write = writes.emplace_back();
			write.slot = bindless_write.slot;
			write.count = 1;
			write.array_index = out_array_indices[i];
			write.type = bindless_write.type;
			write.buffer = bindless_write.buffer;
			write.set = bindless_write.set;
		}

		context->push_descriptor_writes(writes);
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

	int32_t DescriptorBindingTable::get_new(uint32_t slot)
	{
		assert(binding_table.contains(slot));

		if (binding_table[slot].free_indices.empty())
		{
			return -1;
		}

		int32_t new_index = binding_table[slot].free_indices.back();
		binding_table[slot].free_indices.pop_back();
		binding_table[slot].bound_indices.push_back(new_index);

		return new_index;
	}

	void DescriptorBindingTable::free(uint32_t slot, int32_t index)
	{
		assert(binding_table.contains(slot));

		binding_table[slot].free_indices.push_back(index);
		binding_table[slot].bound_indices.erase(
			std::remove(binding_table[slot].bound_indices.begin(), binding_table[slot].bound_indices.end(), index),
			binding_table[slot].bound_indices.end()
		);
	}

	void DescriptorBindingTable::reset(uint32_t slot)
	{
		assert(binding_table.contains(slot));

		binding_table[slot].free_indices.clear();
		binding_table[slot].bound_indices.clear();

		binding_table[slot].free_indices.reserve(binding_table[slot].total_bindings_count);
		for (uint32_t i = binding_table[slot].total_bindings_count - 1; i >= 0; --i)
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
