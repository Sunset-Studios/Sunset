#include <utility/strings.h>

namespace Sunset
{
#ifndef NDEBUG
	std::unordered_map<uint32_t, std::string> g_string_table;
#endif
}
