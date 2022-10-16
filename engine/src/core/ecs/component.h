#pragma once

#include <minimal.h>
#include <bitset>

namespace Sunset
{
	const int MAX_COMPONENTS = 32;

	using ComponentMask = std::bitset<MAX_COMPONENTS>;

	extern int g_component_counter;

	template<class T>
	int get_component_id()
	{
		static int component_id = g_component_counter++;
		return component_id;
	}
}
