#include <core/subsystems/camera_control_processor.h>
#include <core/ecs/components/camera_control_component.h>
#include <core/layers/scene.h>
#include <graphics/renderer.h>
#include <graphics/resource/buffer.h>

#include <glm/gtc/matrix_transform.hpp>

#define FRUSTUM_LEFT 0
#define FRUSTUM_RIGHT 1
#define FRUSTUM_TOP 2
#define FRUSTUM_BOTTOM 3
#define FRUSTUM_BACK 4
#define FRUSTUM_FRONT 5

namespace Sunset
{
	void CameraControlProcessor::update(class Scene* scene, double delta_time)
	{
		const size_t min_ubo_alignment = Renderer::get()->context()->get_min_ubo_offset_alignment();

		for (EntityID entity : SceneView<CameraControlComponent>(*scene))
		{
			CameraControlComponent* const camera_control_comp = scene->get_component<CameraControlComponent>(entity);

			if (camera_control_comp->data.b_dirty)
			{
				camera_control_comp->data.gpu_data.view_matrix = glm::lookAt(
					camera_control_comp->data.position,
					camera_control_comp->data.position + camera_control_comp->data.forward,
					WORLD_UP
				);
				camera_control_comp->data.gpu_data.projection_matrix = glm::perspective(
					glm::radians(camera_control_comp->data.fov),
					camera_control_comp->data.aspect_ratio,
					camera_control_comp->data.near_plane,
					camera_control_comp->data.far_plane
				);
				camera_control_comp->data.gpu_data.projection_matrix[1][1] *= -1;

				camera_control_comp->data.gpu_data.view_projection_matrix = camera_control_comp->data.gpu_data.projection_matrix * camera_control_comp->data.gpu_data.view_matrix;
				camera_control_comp->data.gpu_data.inverse_view_projection_matrix = glm::inverse(camera_control_comp->data.gpu_data.view_projection_matrix);

				// Update camera frustum planes
				{
					const glm::mat4& mvp = camera_control_comp->data.gpu_data.view_projection_matrix;

					camera_control_comp->data.gpu_data.frustum_planes[FRUSTUM_LEFT].x = mvp[0].w + mvp[0].x;
					camera_control_comp->data.gpu_data.frustum_planes[FRUSTUM_LEFT].y = mvp[1].w + mvp[1].x;
					camera_control_comp->data.gpu_data.frustum_planes[FRUSTUM_LEFT].z = mvp[2].w + mvp[2].x;
					camera_control_comp->data.gpu_data.frustum_planes[FRUSTUM_LEFT].w = mvp[3].w + mvp[3].x;

					camera_control_comp->data.gpu_data.frustum_planes[FRUSTUM_RIGHT].x = mvp[0].w - mvp[0].x;
					camera_control_comp->data.gpu_data.frustum_planes[FRUSTUM_RIGHT].y = mvp[1].w - mvp[1].x;
					camera_control_comp->data.gpu_data.frustum_planes[FRUSTUM_RIGHT].z = mvp[2].w - mvp[2].x;
					camera_control_comp->data.gpu_data.frustum_planes[FRUSTUM_RIGHT].w = mvp[3].w - mvp[3].x;

					camera_control_comp->data.gpu_data.frustum_planes[FRUSTUM_TOP].x = mvp[0].w - mvp[0].y;
					camera_control_comp->data.gpu_data.frustum_planes[FRUSTUM_TOP].y = mvp[1].w - mvp[1].y;
					camera_control_comp->data.gpu_data.frustum_planes[FRUSTUM_TOP].z = mvp[2].w - mvp[2].y;
					camera_control_comp->data.gpu_data.frustum_planes[FRUSTUM_TOP].w = mvp[3].w - mvp[3].y;

					camera_control_comp->data.gpu_data.frustum_planes[FRUSTUM_BOTTOM].x = mvp[0].w + mvp[0].y;
					camera_control_comp->data.gpu_data.frustum_planes[FRUSTUM_BOTTOM].y = mvp[1].w + mvp[1].y;
					camera_control_comp->data.gpu_data.frustum_planes[FRUSTUM_BOTTOM].z = mvp[2].w + mvp[2].y;
					camera_control_comp->data.gpu_data.frustum_planes[FRUSTUM_BOTTOM].w = mvp[3].w + mvp[3].y;

					camera_control_comp->data.gpu_data.frustum_planes[FRUSTUM_BACK].x = mvp[0].w + mvp[0].z;
					camera_control_comp->data.gpu_data.frustum_planes[FRUSTUM_BACK].y = mvp[1].w + mvp[1].z;
					camera_control_comp->data.gpu_data.frustum_planes[FRUSTUM_BACK].z = mvp[2].w + mvp[2].z;
					camera_control_comp->data.gpu_data.frustum_planes[FRUSTUM_BACK].w = mvp[3].w + mvp[3].z;

					camera_control_comp->data.gpu_data.frustum_planes[FRUSTUM_FRONT].x = mvp[0].w - mvp[0].z;
					camera_control_comp->data.gpu_data.frustum_planes[FRUSTUM_FRONT].y = mvp[1].w - mvp[1].z;
					camera_control_comp->data.gpu_data.frustum_planes[FRUSTUM_FRONT].z = mvp[2].w - mvp[2].z;
					camera_control_comp->data.gpu_data.frustum_planes[FRUSTUM_FRONT].w = mvp[3].w - mvp[3].z;

					for (uint8_t i = 0; i < 6; ++i)
					{
						const float length = glm::length(glm::vec3(camera_control_comp->data.gpu_data.frustum_planes[i]));
						camera_control_comp->data.gpu_data.frustum_planes[i] /= length;
					}
				}

				camera_control_comp->data.b_dirty = false;

				for (int i = 0; i < MAX_BUFFERED_FRAMES; ++i)
				{
					CACHE_FETCH(Buffer, scene->scene_data.buffer)->copy_from(
						Renderer::get()->context(),
						&camera_control_comp->data.gpu_data,
						sizeof(CameraData),
						scene->scene_data.cam_data_buffer_start + BufferHelpers::pad_ubo_size(sizeof(CameraData), min_ubo_alignment) * i
					);
				}
			}

			// TODO: Only do this for the active camera
			const glm::mat4 projection = camera_control_comp->data.gpu_data.projection_matrix;

			DrawCullData new_draw_cull_data
			{
				.p00 = projection[0][0],
				.p11 = projection[1][1],
				.culling_enabled = true,
				.occlusion_enabled = true,
				.distance_check = true
			};
			
			// Queueing this up as a render graph command as the renderer will eventually be on a separate thread, and we want some ordering
			// guarantee as to when this data first gets set for re-use in a later render graph pass
			QUEUE_RENDERGRAPH_COMMAND(SetDrawCullData, [new_draw_cull_data](class RenderGraph& render_graph, RGFrameData& frame_data, void* command_buffer)
			{
				Renderer::get()->set_draw_cull_data(new_draw_cull_data);
			});
		}
	}
}
