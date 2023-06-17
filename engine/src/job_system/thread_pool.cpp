#include <job_system/thread_pool.h>

namespace Sunset
{
	ThreadPool::ThreadPool()
	{
		num_available_threads = std::thread::hardware_concurrency();
		threads.resize(num_available_threads);
	}

	ThreadPool::ThreadPool(uint32_t max_num_threads)
	{
		num_available_threads = std::min(std::thread::hardware_concurrency(), max_num_threads);
		threads.resize(num_available_threads);
	}

	void ThreadPool::join_all()
	{
		static_cast<void>(threads.empty());
	}
}
