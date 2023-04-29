#pragma once

#include <minimal.h>
#include <gpu_shared_data_types.h>

namespace Sunset
{
	constexpr uint32_t MAX_LIGHT_COUNT = 1000;

	enum class LightingModel : uint32_t
	{
		BlinnPhong = 0,
		Standard = 1
	};

	enum class LightType : uint32_t
	{
		Directional = 0,
		Point = 1,
		Spot = 2,
		Area = 3
	};

	struct alignas(16) LightData
	{
		glm::vec4 color;       // r,g,b,a - a=1.0 for active, a=0.0 for inactive
		glm::vec4 direction;   // Direction vector for where the light points (directional and spot). x, y, z, w - w=inner_angle
		float radius;          // Light radius for point
		float outer_angle;     // Outer angle for spot lights (in radians)
		glm::vec2 area_size;   // Width and height for area lights
		LightType type;        // Light type
		uint32_t entity_index; // Index of object entity
		float intensity;       // Intensity of the light
	};

	DECLARE_GPU_SHARED_DATA(LightData, MAX_LIGHT_COUNT);

	struct Light
	{
		LightData* gpu_data{ nullptr };
		uint32_t gpu_data_buffer_offset{ 0 };
		bool b_dirty{ true };

		void destroy(class GraphicsContext* const context) { }
	};
}
