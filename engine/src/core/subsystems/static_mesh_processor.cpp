#include <core/subsystems/static_mesh_processor.h>
#include <core/ecs/components/mesh_component.h>
#include <core/layers/scene.h>

namespace Sunset
{
	void StaticMeshProcessor::update(class Scene* scene, double delta_time)
	{
		for (EntityID entity : SceneView<MeshComponent>(*scene))
		{
			MeshComponent* const mesh_comp = scene->get_component<MeshComponent>(entity);

			// Setup and submit a render task for this mesh
			// * Get new or cached pipeline state
			// * Get new or cached resource state
			// * Create a render task and submit
		}
	}
}
