#pragma once

#include <minimal.h>
#include <core/subsystem.h>

namespace Sunset
{
	class LightProcessor : public Subsystem
	{
	public:
		LightProcessor() = default;
		~LightProcessor() = default;

		virtual void initialize(class Scene* scene) override;
		virtual void destroy(class Scene* scene) override { };
		virtual void update(class Scene* scene, double delta_time) override;

		void calculate_csm_matrices(class Scene* scene, class CameraControlComponent* camera_comp, const glm::vec3& light_dir, uint32_t buffered_frame_number);
		glm::mat4 calculate_light_space_matrix(class CameraControlComponent* camera_comp, float near, float far, const glm::vec3& light_dir, uint32_t buffered_frame_number);
		std::array<glm::vec3, 8> get_world_space_frustum_corners(class CameraControlComponent* camera_comp, float near, float far);
	};
}
