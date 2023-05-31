#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include <core/ecs/entity.h>

#include <array>

namespace Sunset
{
	struct alignas(16) CameraData
	{
		glm::mat4 view_matrix;
		glm::mat4 projection_matrix;
		glm::mat4 view_projection_matrix;
		glm::mat4 prev_view_projection_matrix;
		glm::mat4 inverse_view_projection_matrix;
		glm::vec4 frustum_planes[6];
		glm::vec4 position;
		glm::vec4 jitter{ 0.0f, 0.0f, 0.0f, 0.0f };
		float fov;
		float padding[3];
	};

	struct CameraTransformData
	{
		float fov;
		float aspect_ratio;
		float near_plane;
		float far_plane;
		float yaw{ -90.0f };
		float pitch{ 0.0f };
		glm::vec3 prev_position;
		glm::vec3 position;
		glm::vec3 prev_forward{ 0.0f, 0.0f, -1.0f };
		glm::vec3 forward{ 0.0f, 0.0f, -1.0f };
		CameraData gpu_data;
		bool b_frame_jitter{ true };
		uint32_t current_jitter_index{ 0 };
	};

	struct CameraInputData
	{
		float move_speed;
		float look_speed;
	};

	struct CameraControlComponent
	{
		CameraTransformData data;
		CameraInputData input;
	};

	void set_fov(CameraControlComponent* camera_comp, float new_fov);
	void set_aspect_ratio(CameraControlComponent* camera_comp, float new_aspect_ratio);
	void set_near_plane(CameraControlComponent* camera_comp, float new_near_plane);
	void set_far_plane(CameraControlComponent* camera_comp, float new_far_plane);
	void set_position(CameraControlComponent* camera_comp, const glm::vec3& new_position);
	void set_forward(CameraControlComponent* camera_comp, const glm::vec3& new_forward);
	void set_move_speed(CameraControlComponent* camera_comp, float new_move_speed);
	void set_look_speed(CameraControlComponent* camera_comp, float new_look_speed);
	void set_frame_jitter_enabled(CameraControlComponent* camera_comp, bool b_enabled);

	void set_camera_fov(class Scene* scene, EntityID entity, float new_fov);
	void set_camera_aspect_ratio(class Scene* scene, EntityID entity, float new_aspect_ratio);
	void set_camera_near_plane(class Scene* scene, EntityID entity, float new_near_plane);
	void set_camera_far_plane(class Scene* scene, EntityID entity, float new_far_plane);
	void set_camera_position(class Scene* scene, EntityID entity, const glm::vec3& new_position);
	void set_camera_forward(class Scene* scene, EntityID entity, const glm::vec3& new_forward);
	void set_camera_move_speed(class Scene* scene, EntityID entity, float new_move_speed);
	void set_camera_look_speed(class Scene* scene, EntityID entity, float new_look_speed);
	void set_frame_jitter_enabled(class Scene* scene, EntityID entity, bool b_enabled);
}
