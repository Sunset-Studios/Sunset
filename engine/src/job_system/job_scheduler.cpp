#include <job_system/job_scheduler.h>

namespace Sunset
{
	std::vector<JobQueue> JobScheduler::per_thread_queues;

	JobScheduler::JobScheduler()
		: thread_pool(MAX_SCHEDULER_THREADS)
	{
		per_thread_queues = std::vector<JobQueue>(thread_pool.get_num_threads());
	}

	void JobScheduler::initialize()
	{
		if (!b_initialized.load())
		{
			const uint32_t num_threads = thread_pool.get_num_threads();
			for (uint32_t i = 0; i < num_threads; ++i)
			{
				thread_pool.run_on_thread([thread_index = i](std::stop_token st, JobScheduler* scheduler)
				{
					while (!st.stop_requested())
					{
						JobQueue* const thread_queue = JobScheduler::get_job_queue(thread_index);
						if (thread_queue != nullptr && !thread_queue->was_empty())
						{
							Job::Handle job = thread_queue->pop();
							job.resume();
						}
					}
				}, i, this);
			}
			b_initialized.store(true);
		}
	}

	void JobScheduler::schedule(Job::Handle ch, bool b_thread_remain)
	{
		uint32_t min_queue_size = std::numeric_limits<uint32_t>::max();
		int32_t smallest_queue{ -1 };

		if (b_thread_remain && ch.promise().thread_index != -1)
		{
			assert(ch.promise().thread_index < per_thread_queues.size());
			per_thread_queues[ch.promise().thread_index].push(ch);
			return;
		}

		for (uint32_t i = 0; i < per_thread_queues.size(); ++i)
		{
			const uint32_t queue_size = per_thread_queues[i].was_size();
			if (queue_size < min_queue_size)
			{
				min_queue_size = queue_size;
				smallest_queue = i;
			}
		}

		if (smallest_queue != -1)
		{
			ch.promise().thread_index = smallest_queue;
			per_thread_queues[smallest_queue].push(ch);
		}
	}

	std::suspend_always Job::promise_type::initial_suspend() noexcept
	{
		JobScheduler::get()->schedule(Job::Handle::from_promise(*this));
		return {};
	}
}
