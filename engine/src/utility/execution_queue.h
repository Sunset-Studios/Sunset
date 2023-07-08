#pragma once

#include <functional>
#include <vector>

namespace Sunset
{
	constexpr uint32_t MIN_EXECUTION_QUEUE_SIZE = 1024;

	struct ExecutionQueue
	{
		ExecutionQueue()
		{
			executions.reserve(MIN_EXECUTION_QUEUE_SIZE);
			execution_ids.reserve(MIN_EXECUTION_QUEUE_SIZE);
			execution_delays.reserve(MIN_EXECUTION_QUEUE_SIZE);
		}

		void push_execution(const std::function<void()>& execution, size_t execution_id = 0, uint32_t execution_frame_delay = 0)
		{
			executions.push_back(execution);
			execution_ids.push_back(execution_id);
			execution_delays.push_back(execution_frame_delay);
		}

		void remove_execution(size_t execution_id)
		{
			if (const auto it = std::find(execution_ids.begin(), execution_ids.end(), execution_id); it != execution_ids.end())
			{
				const uint32_t index = std::distance(execution_ids.begin(), it);
				executions.erase(executions.begin() + index);
				execution_ids.erase(execution_ids.begin() + index);
				execution_delays.erase(execution_delays.begin() + index);
			}
		}

		void update()
		{
			for (int32_t i = executions.size() - 1; i >= 0; --i)
			{
				if (--execution_delays[i] < 0)
				{
					(executions[i])();
					executions.erase(executions.begin() + i);
					execution_ids.erase(execution_ids.begin() + i);
					execution_delays.erase(execution_delays.begin() + i);
				}
			}
		}

		void flush()
		{
			for (auto it = executions.rbegin(); it != executions.rend(); ++it)
			{
				(*it)();
			}
			executions.clear();
			execution_ids.clear();
			execution_delays.clear();
		}

		std::vector<std::function<void()>> executions;
		std::vector<size_t> execution_ids;
		std::vector<uint32_t> execution_delays;
	};
}
