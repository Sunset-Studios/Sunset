#include <job_system/job_scheduler.h>
#include <utility/cvar.h>

namespace Sunset
{
	AutoCVar_Int cvar_idle_thread_nanosecond_sleep("jobs.idle_thread_nanosecond_sleep", "Number of nanoseconds to wait between sleeps when job scheduler threads have no work queued", 5);

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
						else
						{
							std::this_thread::sleep_for(std::chrono::nanoseconds(cvar_idle_thread_nanosecond_sleep.get()));
						}
					}
				}, i, this);
			}
			b_initialized.store(true);
		}
	}

	int32_t JobScheduler::schedule(Job::Handle ch, uint32_t thread_index, bool b_thread_remain, bool b_initial_run)
	{
		uint32_t min_queue_size = std::numeric_limits<uint32_t>::max();
		int32_t smallest_queue{ -1 };

		if ((b_thread_remain || b_initial_run) && thread_index != -1)
		{
			assert(thread_index < per_thread_queues.size());
			per_thread_queues[thread_index].push(ch);
			return thread_index;
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
			per_thread_queues[smallest_queue].push(ch);
		}

		return smallest_queue;
	}

	Job::Job(const std::coroutine_handle<>& handle)
		: handle(handle)
	{
		JobScheduler::get()->set_job_done_state(handle.address(), false);
	}

	bool Job::is_done() const
	{
		return JobScheduler::get()->get_job_done_state(handle.address());
	}

	void parallel_for(uint32_t iterations, std::function<void(uint32_t)> op)
	{
		const auto parallel_op = [op](uint32_t index) -> ThreadedJob<>
		{
			op(index);
			co_return;
		};

		JobBatcher<ThreadedJob<>> jobs(iterations);
		for (uint32_t i = 0; i < iterations; ++i)
		{
			jobs.add(parallel_op(i), i);
		}

		{
			ZoneScopedN("parallel_for: wait_on_all");
			jobs.wait_on_all();
		}
	}
}
