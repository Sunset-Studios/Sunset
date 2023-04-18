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
		float max_depth{ 1.0f };

		bool operator==(const Viewport& other) const
		{
			return x == other.x && y == other.y
				&& width == other.width && height == other.height
				&& min_depth == other.min_depth && max_depth == other.max_depth;
		}
	};
	
	struct Scissor
	{
		int x{ 0 };
		int y{ 0 };
		int w{ 0 };
		int h{ 0 };

		bool operator==(const Scissor& other) const
		{
			return x == other.x && y == other.y && w == other.w && h == other.h;
		}
	};
}
