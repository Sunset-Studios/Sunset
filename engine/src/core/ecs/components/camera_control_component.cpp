#include <core/ecs/components/camera_control_component.h>
#include <core/layers/scene.h>

namespace Sunset
{
	void set_fov(CameraControlComponent* camera_comp, float new_fov)
	{
		camera_comp->data.fov = new_fov;
		camera_comp->data.b_dirty = true;
	}

	void set_camera_fov(class Scene* scene, EntityID entity, float new_fov)
	{
		if (CameraControlComponent* const camera_comp = scene->get_component<CameraControlComponent>(entity))
		{
			set_fov(camera_comp, new_fov);
		}
	}

	void set_aspect_ratio(CameraControlComponent* camera_comp, float new_aspect_ratio)
	{
		camera_comp->data.aspect_ratio = new_aspect_ratio;
		camera_comp->data.b_dirty = true;
	}

	void set_camera_aspect_ratio(class Scene* scene, EntityID entity, CameraControlComponent* camera_comp, float new_aspect_ratio)
	{
		if (CameraControlComponent* const camera_comp = scene->get_component<CameraControlComponent>(entity))
		{
			set_aspect_ratio(camera_comp, new_aspect_ratio);
		}
	}

	void set_near_plane(CameraControlComponent* camera_comp, float new_near_plane)
	{
		camera_comp->data.near_plane = new_near_plane;
		camera_comp->data.b_dirty = true;
	}

	void set_camera_near_plane(class Scene* scene, EntityID entity, CameraControlComponent* camera_comp, float new_near_plane)
	{
		if (CameraControlComponent* const camera_comp = scene->get_component<CameraControlComponent>(entity))
		{
			set_near_plane(camera_comp, new_near_plane);
		}
	}

	void set_far_plane(CameraControlComponent* camera_comp, float new_far_plane)
	{
		camera_comp->data.far_plane = new_far_plane;
		camera_comp->data.b_dirty = true;
	}

	void set_camera_far_plane(class Scene* scene, EntityID entity, CameraControlComponent* camera_comp, float new_far_plane)
	{
		if (CameraControlComponent* const camera_comp = scene->get_component<CameraControlComponent>(entity))
		{
			set_far_plane(camera_comp, new_far_plane);
		}
	}

	void set_position(CameraControlComponent* camera_comp, const glm::vec3& new_position)
	{
		camera_comp->data.prev_position = camera_comp->data.position;
		camera_comp->data.position = new_position;
		camera_comp->data.b_dirty = true;
	}

	void set_camera_position(class Scene* scene, EntityID entity, CameraControlComponent* camera_comp, const glm::vec3& new_position)
	{
		if (CameraControlComponent* const camera_comp = scene->get_component<CameraControlComponent>(entity))
		{
			set_position(camera_comp, new_position);
		}
	}

	void set_forward(CameraControlComponent* camera_comp, const glm::vec3& new_forward)
	{
		camera_comp->data.prev_forward = camera_comp->data.forward;
		camera_comp->data.forward = new_forward;
		camera_comp->data.b_dirty = true;
	}

	void set_camera_forward(class Scene* scene, EntityID entity, const glm::vec3& new_forward)
	{
		if (CameraControlComponent* const camera_comp = scene->get_component<CameraControlComponent>(entity))
		{
			set_forward(camera_comp, new_forward);
		}
	}

	void set_move_speed(CameraControlComponent* camera_comp, float new_move_speed)
	{
		camera_comp->input.move_speed = new_move_speed;
	}

	void set_look_speed(CameraControlComponent* camera_comp, float new_look_speed)
	{
		camera_comp->input.look_speed = new_look_speed;
	}

	void set_camera_move_speed(class Scene* scene, EntityID entity, CameraControlComponent* camera_comp, float new_move_speed)
	{
		if (CameraControlComponent* const camera_comp = scene->get_component<CameraControlComponent>(entity))
		{
			set_move_speed(camera_comp, new_move_speed);
		}
	}

	void set_camera_look_speed(class Scene* scene, EntityID entity, float new_look_speed)
	{
		if (CameraControlComponent* const camera_comp = scene->get_component<CameraControlComponent>(entity))
		{
			set_look_speed(camera_comp, new_look_speed);
		}
	}
}
