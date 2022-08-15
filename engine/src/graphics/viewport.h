#pragma once

namespace Sunset
{
	struct Viewport
	{
		float x{ 0.0f };
		float y{ 0.0f };
		float width{ 0.0f };
		float height{ 0.0f };
		float min_depth{ 0.0f };
		float max_depth{ 0.0f };
	};
	
	struct Scissor
	{
		int x{ 0 };
		int y{ 0 };
		int w{ 0 };
		int h{ 0 };
	};
}
