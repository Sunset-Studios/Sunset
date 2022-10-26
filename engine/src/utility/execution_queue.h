#pragma once

#include <functional>
#include <vector>

namespace Sunset
{
	struct ExecutionQueue
	{
		void push_execution(const std::function<void()>& execution)
		{
			executions.push_back(execution);
		}

		void flush()
		{
			for (auto it = executions.rbegin(); it != executions.rend(); ++it)
			{
				(*it)();
			}
			executions.clear();
		}

		std::vector<std::function<void()>> executions;
	};
}
