#pragma once

#include <minimal.h>
#include <singleton.h>
#include <entity.h>

namespace Sunset
{
	struct EntityTransformData
	{
		BufferID transform_buffer[MAX_BUFFERED_FRAMES];
		std::array<glm::mat4, MIN_ENTITIES> entity_transforms;
	};

	class EntityGlobals : public Singleton<EntityGlobals>
	{
	friend class Singleton;

	public:
		void initialize() { }

	public:
		EntityTransformData transforms;
	};
}
