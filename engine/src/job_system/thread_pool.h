#pragma once

#include <minimal.h>

namespace Sunset
{
	class ThreadPool
	{
	public:
		ThreadPool();
		explicit ThreadPool(uint32_t max_num_threads);
		ThreadPool(const ThreadPool&) = delete;
		ThreadPool& operator=(const ThreadPool&) = delete;

		uint32_t get_num_threads() const noexcept
		{
			return num_available_threads;
		}

		template<typename F, typename... Args>
		void run_on_thread(F&& lambda, uint32_t thread_index, Args&&... args)
		{
			assert(thread_index >= 0 && thread_index < num_available_threads);
			threads[thread_index] = std::jthread(lambda, std::forward<Args>(args)...);
		}

		void join_all();
		
	protected:
		uint32_t num_available_threads{ 0 };
		std::vector<std::jthread> threads;
	};
}
