#pragma once

#include <minimal.h>
#include <bit_vector.h>

namespace Sunset
{
	constexpr int MAX_COMPONENTS = 32;

	using ComponentMask = BitVector<MAX_COMPONENTS>;

	extern int g_component_counter;

	template<class T>
	int get_component_id()
	{
		static int component_id = g_component_counter++;
		return component_id;
	}
}
