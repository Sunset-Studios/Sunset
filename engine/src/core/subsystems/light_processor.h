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
		glm::mat4 calculate_light_space_matrix(float fov, float aspect_ratio, float near, float far, const glm::mat4& view, const glm::vec3& light_dir);
		std::array<glm::vec4, 8> get_world_space_frustum_corners(const glm::mat4& inv_view_proj);
	};
}
