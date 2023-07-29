#include <core/subsystems/light_processor.h>
#include <core/layers/scene.h>
#include <core/data_globals.h>
#include <core/ecs/components/light_component.h>
#include <core/ecs/components/transform_component.h>
#include <core/ecs/components/camera_control_component.h>
#include <graphics/resource/buffer.h>
#include <graphics/renderer.h>
#include <graphics/debug_draw_helpers.h>
#include <utility/cvar.h>

namespace Sunset
{
	AutoCVar_Bool cvar_light_to_frustum_split_line_draw("ren.debug.light_to_frustum_split_line_draw", "Whether or not to draw a line from the center of every frustum split to the CSM affecting light source", false);
	AutoCVar_Float cvar_csm_camera_frustum_inflation("ren.csm.camera_frustum_inflation", "How much to scale the camera frustum when computing the tight fitting light space matrices", 1.5f);

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
		static float frustum_split_percentages[MAX_SHADOW_CASCADES] = { 0.02f, 0.04f, 0.1f, 0.2f };
		ZoneScopedN("LightProcessor::calculate_csm_matrices");
		for (uint32_t index = 0; index < MAX_SHADOW_CASCADES; ++index)
		{
			ZoneScopedN("LightProcessor::calculate_csm_matrices iteration");
			const float near_plane = index == 0 ? camera_comp->data.near_plane : camera_comp->data.far_plane * frustum_split_percentages[index - 1];
			const float far_plane = camera_comp->data.far_plane * frustum_split_percentages[index];
			scene->scene_data.lighting[buffered_frame_number].csm_plane_distances[index] = far_plane;
			scene->scene_data.lighting[buffered_frame_number].light_space_matrices[index] = calculate_light_space_matrix(
				camera_comp,
				near_plane,
				far_plane,
				glm::normalize(light_dir),
				buffered_frame_number
			);
		};
	}

	glm::mat4 LightProcessor::calculate_light_space_matrix(CameraControlComponent* camera_comp, float near, float far, const glm::vec3& light_dir, uint32_t buffered_frame_number)
	{
		ZoneScopedN("LightProcessor::calculate_light_space_matrix");

		const float frustum_inflation = cvar_csm_camera_frustum_inflation.get();

		glm::vec3 center;
		std::array<glm::vec4, 8> corners = get_world_space_frustum_corners(camera_comp, near, far, center, frustum_inflation);

		if (cvar_light_to_frustum_split_line_draw.get())
		{
			debug_draw_line(Renderer::get()->context(), corners[0], corners[1], glm::vec3(1.0f, 0.0f, 0.0f), buffered_frame_number);
			debug_draw_line(Renderer::get()->context(), corners[1], corners[2], glm::vec3(1.0f, 0.0f, 0.0f), buffered_frame_number);
			debug_draw_line(Renderer::get()->context(), corners[2], corners[3], glm::vec3(1.0f, 0.0f, 0.0f), buffered_frame_number);
			debug_draw_line(Renderer::get()->context(), corners[3], corners[0], glm::vec3(1.0f, 0.0f, 0.0f), buffered_frame_number);
			debug_draw_line(Renderer::get()->context(), corners[0], corners[4], glm::vec3(1.0f, 0.0f, 0.0f), buffered_frame_number);
			debug_draw_line(Renderer::get()->context(), corners[1], corners[5], glm::vec3(1.0f, 0.0f, 0.0f), buffered_frame_number);
			debug_draw_line(Renderer::get()->context(), corners[2], corners[6], glm::vec3(1.0f, 0.0f, 0.0f), buffered_frame_number);
			debug_draw_line(Renderer::get()->context(), corners[3], corners[7], glm::vec3(1.0f, 0.0f, 0.0f), buffered_frame_number);
			debug_draw_line(Renderer::get()->context(), corners[4], corners[5], glm::vec3(1.0f, 0.0f, 0.0f), buffered_frame_number);
			debug_draw_line(Renderer::get()->context(), corners[5], corners[6], glm::vec3(1.0f, 0.0f, 0.0f), buffered_frame_number);
			debug_draw_line(Renderer::get()->context(), corners[6], corners[7], glm::vec3(1.0f, 0.0f, 0.0f), buffered_frame_number);
			debug_draw_line(Renderer::get()->context(), corners[7], corners[4], glm::vec3(1.0f, 0.0f, 0.0f), buffered_frame_number);
			debug_draw_line(Renderer::get()->context(), center, center - light_dir, glm::vec3(1.0f, 0.0f, 0.0f), buffered_frame_number);
		}

		center.y = camera_comp->data.position.y; // Prevents an offset to the shadow map cascade when light view matrix tries to follow a downward facing camera frustum
		const glm::mat4 light_view = glm::lookAt(center + light_dir * (far - near) * frustum_inflation, center, WORLD_UP);

		glm::vec3 min(std::numeric_limits<float>::max());
		glm::vec3 max(std::numeric_limits<float>::lowest());
		for (const glm::vec4& corner : corners)
		{
			const glm::vec3 lv_corner = light_view * corner;
			min = glm::min(min, lv_corner);
			max = glm::max(max, lv_corner);
		}

		const float radius = glm::length(max - min) * 0.5f;

		glm::mat4 ortho = glm::ortho(-radius, radius, -radius, radius, -radius, radius);
		ortho[1][1] *= -1;

		return ortho * light_view;
	}

	std::array<glm::vec4, 8> LightProcessor::get_world_space_frustum_corners(CameraControlComponent* camera_comp, float near, float far, glm::vec3& out_center, float inflation_factor)
	{
		glm::mat3 basis
		{
			glm::normalize(camera_comp->data.gpu_data.view_matrix[0]),
			glm::normalize(camera_comp->data.gpu_data.view_matrix[1]),
			glm::normalize(camera_comp->data.gpu_data.view_matrix[2])
		};

		const float tang = glm::tan(camera_comp->data.gpu_data.fov * 0.5f);
		const float near_height = near * tang * inflation_factor;
		const float near_width = near_height * camera_comp->data.aspect_ratio;
		const float far_height = far * tang * inflation_factor;
		const float far_width = far_height * camera_comp->data.aspect_ratio;

		glm::vec3 near_center = camera_comp->data.position + basis[2] * near;
		glm::vec3 far_center = camera_comp->data.position + basis[2] * far;

		std::array<glm::vec4, 8> corners;

		corners[0] = glm::vec4(near_center + basis[1] * near_height - basis[0] * near_width, 1.0f);
		corners[1] = glm::vec4(near_center + basis[1] * near_height + basis[0] * near_width, 1.0f);
		corners[2] = glm::vec4(near_center - basis[1] * near_height + basis[0] * near_width, 1.0f);
		corners[3] = glm::vec4(near_center - basis[1] * near_height - basis[0] * near_width, 1.0f);
		corners[4] = glm::vec4(far_center + basis[1] * far_height - basis[0] * far_width, 1.0f);
		corners[5] = glm::vec4(far_center + basis[1] * far_height + basis[0] * far_width, 1.0f);
		corners[6] = glm::vec4(far_center - basis[1] * far_height + basis[0] * far_width, 1.0f);
		corners[7] = glm::vec4(far_center - basis[1] * far_height - basis[0] * far_width, 1.0f);

		out_center = (near_center + far_center) * 0.5f;

		return corners;
	}
}
