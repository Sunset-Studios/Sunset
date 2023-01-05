#pragma once

#include <core/simulation_layer.h>

namespace Sunset
{
	class EditorGui : public SimulationLayer
	{
	public:
		EditorGui() = default;
		EditorGui(const EditorGui&) = delete;
		EditorGui& operator=(const EditorGui&) = delete;
		~EditorGui() = default;

		virtual void initialize() override;
		virtual void destroy() override;
		virtual void update(double delta_time) override;
	};
}
