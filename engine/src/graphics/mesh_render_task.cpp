#include <graphics/mesh_render_task.h>
#include <graphics/render_pass.h>
#include <graphics/renderer.h>
#include <graphics/pipeline_state.h>
#include <graphics/resource_state.h>
#include <graphics/graphics_context.h>
#include <graphics/descriptor.h>
#include <graphics/resource/material.h>

namespace Sunset
{
	MeshRenderTask* MeshRenderTask::setup(MaterialID new_material, ResourceStateID new_resource_state, uint32_t new_render_depth /*= 0*/)
	{
		material = new_material;
		resource_state = new_resource_state;
		render_depth = new_render_depth;
		return this;
	}

	void MeshRenderTask::submit(MeshTaskQueue& queue)
	{
		task_hash = std::hash<MeshRenderTask>{}(*this);
		queue.add(this);
	}

	void MeshRenderTaskExecutor::reset()
	{
		cached_material = 0;
		cached_resource_state = 0;
	}
}
