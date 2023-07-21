#include <core/subsystems/light_processor.h>
#include <core/layers/scene.h>
#include <core/data_globals.h>
#include <core/ecs/components/light_component.h>
#include <core/ecs/components/transform_component.h>
#include <core/ecs/components/camera_control_component.h>
#include <graphics/resource/buffer.h>
#include <graphics/renderer.h>

namespace Sunset
{
	void LightProcessor::initialize(class Scene* scene)
	{
		for (int i = 0; i < MAX_BUFFERED_FRAMES; ++i)
		{
			if (LightGlobals::get()->light_data.data_buffer[i] == 0)
			{
				LightGlobals::get()->light_data.data_buffer[i] = BufferFactory::create(
					Renderer::get()->context(),
					{
						.name = "light_datas",
						.buffer_size = sizeof(LightData) * MIN_ENTITIES,
						.type = BufferType::StorageBuffer
					}
				);
			}
		}
	}

	void LightProcessor::update(class Scene* scene, double delta_time)
	{
		ZoneScopedN("LightProcessor::update");

		GraphicsContext* const gfx_context = Renderer::get()->context();
		const uint32_t current_buffered_frame = gfx_context->get_buffered_frame_number();

		CameraControlComponent* const camera_comp = scene->get_component<CameraControlComponent>(scene->active_camera);

		scene->scene_data.lighting[current_buffered_frame].num_lights = 0;

		for (EntityID entity : SceneView<LightComponent, TransformComponent>(*scene))
		{
			LightComponent* const light_comp = scene->get_component<LightComponent>(entity);
			TransformComponent* const transform_comp = scene->get_component<TransformComponent>(entity);

			const int32_t entity_index = get_entity_index(entity);

			EntitySceneData& entity_data = EntityGlobals::get()->entity_data[entity_index];

			if (LightGlobals::get()->light_dirty_states.test(light_comp->light_data_buffer_offset))
			{
				const glm::vec4 light_position = transform_comp->transform.local_matrix[3];
				entity_data.bounds_pos_radius = glm::vec4(light_position.x, light_position.y, light_position.z, light_comp->light->radius);
				entity_data.local_transform = transform_comp->transform.local_matrix;
				LightGlobals::get()->light_dirty_states.unset(light_comp->light_data_buffer_offset);
			}

			const bool b_light_enabled = light_comp->light->color.a > 0.0f;
			if (b_light_enabled && light_comp->light->b_casts_shadows != 0 && light_comp->light->b_csm_caster != 0)
			{
				const glm::vec4 light_dir = light_comp->light->b_use_sun_directon > 0 ? scene->scene_data.lighting[current_buffered_frame].sunlight_direction : -entity_data.bounds_pos_radius;
				calculate_csm_matrices(
					scene,
					camera_comp,
					light_dir,
					current_buffered_frame
				);
			}

			scene->scene_data.lighting[current_buffered_frame].num_lights += b_light_enabled;
		}

		QUEUE_RENDERGRAPH_COMMAND(CopyLightData, ([](class RenderGraph& render_graph, RGFrameData& frame_data, void* command_buffer)
		{
			// TODO: Only update dirtied entities instead of re-uploading the buffer every frame
			Buffer* const lights_buffer = CACHE_FETCH(Buffer, LightGlobals::get()->light_data.data_buffer[frame_data.buffered_frame_number]);
			lights_buffer->copy_from(
				frame_data.gfx_context,
				LightGlobals::get()->light_data.data.data(),
				LightGlobals::get()->light_data.data.size() * sizeof(LightData)
			);
		}));
	}

	void LightProcessor::calculate_csm_matrices(Scene* scene, CameraControlComponent* camera_comp, const glm::vec3& light_dir, uint32_t buffered_frame_number)
	{
		static float frustum_split_percentages[MAX_SHADOW_CASCADES] = { 0.02f, 0.1f, 1.0f };
		ZoneScopedN("LightProcessor::calculate_csm_matrices");
		for (uint32_t index = 0; index < MAX_SHADOW_CASCADES; ++index)
		{
			ZoneScopedN("LightProcessor::calculate_csm_matrices iteration");
			const float near_plane = index == 0 ? camera_comp->data.near_plane : camera_comp->data.far_plane * frustum_split_percentages[index - 1];
			const float far_plane = camera_comp->data.far_plane * frustum_split_percentages[index];
			scene->scene_data.lighting[buffered_frame_number].csm_plane_distances[index] = far_plane;
			scene->scene_data.lighting[buffered_frame_number].light_space_matrices[index] = calculate_light_space_matrix(
				camera_comp->data.gpu_data.fov,
				camera_comp->data.aspect_ratio,
				near_plane,
				far_plane,
				camera_comp->data.gpu_data.view_matrix,
				light_dir
			);
		};
	}

	glm::mat4 LightProcessor::calculate_light_space_matrix(float fov, float aspect_ratio, float near, float far, const glm::mat4& view, const glm::vec3& light_dir)
	{
		ZoneScopedN("LightProcessor::calculate_light_space_matrix");

		glm::mat4 proj = glm::perspective(fov, aspect_ratio, near, far);
		proj[1][1] *= -1;
		const std::array<glm::vec4, 8> corners = get_world_space_frustum_corners(glm::inverse(proj * view));

		glm::vec3 center(0.0f, 0.0f, 0.0f);
		for (const glm::vec4& corner : corners)
		{
			center.x += corner.x;
			center.y += corner.y;
			center.z += corner.z;
		}
		center /= 8.0f; 

		const glm::mat4 light_view = glm::lookAt(center + light_dir, center, WORLD_UP);

		glm::vec3 min(std::numeric_limits<float>::max());
		glm::vec3 max(std::numeric_limits<float>::lowest());
		for (const glm::vec4& corner : corners)
		{
			const glm::vec3 lv_corner = glm::vec3(light_view * corner);
			min = glm::min(min, lv_corner);
			max = glm::max(max, lv_corner);
		}

		glm::mat4 light_projection = glm::ortho(min.x, max.x, max.y, min.y, -max.z, -min.z);

		return light_projection * light_view;
	}

	std::array<glm::vec4, 8> LightProcessor::get_world_space_frustum_corners(const glm::mat4& inv_view_proj)
	{
		static glm::vec4 corners[8] = {
			{-1, -1, -1, 1},
			{-1, -1, 1, 1},
			{-1, 1, -1, 1},
			{-1, 1, 1, 1},
			{1, -1, -1, 1},
			{1, -1, 1, 1},
			{1, 1, -1, 1},
			{1, 1, 1, 1}
		};

		std::array<glm::vec4, 8> transformed_corners = {
			inv_view_proj * corners[0],
			inv_view_proj * corners[1],
			inv_view_proj * corners[2],
			inv_view_proj * corners[3],
			inv_view_proj * corners[4],
			inv_view_proj * corners[5],
			inv_view_proj * corners[6],
			inv_view_proj * corners[7]
		};
		transformed_corners[0] /= transformed_corners[0].w;
		transformed_corners[1] /= transformed_corners[1].w;
		transformed_corners[2] /= transformed_corners[2].w;
		transformed_corners[3] /= transformed_corners[3].w;
		transformed_corners[4] /= transformed_corners[4].w;
		transformed_corners[5] /= transformed_corners[5].w;
		transformed_corners[6] /= transformed_corners[6].w;
		transformed_corners[7] /= transformed_corners[7].w;

		return transformed_corners;
	}
}
