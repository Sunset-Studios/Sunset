#pragma once

#include <minimal.h>
#include <job_system/thread_pool.h>
#include <utility/pattern/singleton.h>

#include <atomic_queue/atomic_queue.h>

#include <coroutine>

namespace Sunset
{
	struct Job
	{
		struct promise_type
		{
			int32_t thread_index{ -1 };

			Job get_return_object() { return {}; }
			std::suspend_always initial_suspend() noexcept;
			std::suspend_never final_suspend() noexcept { return {}; }
			void unhandled_exception() {}
			void return_void() {}
		};

		using Handle = std::coroutine_handle<promise_type>;
	};

	constexpr uint32_t MAX_JOB_QUEUE_SIZE = 4096;
	constexpr uint32_t MAX_SCHEDULER_THREADS = 32;
	using JobQueue = atomic_queue::AtomicQueue2<Job::Handle, MAX_JOB_QUEUE_SIZE>;

	class JobScheduler : public Singleton<JobScheduler>
	{
		friend class Singleton;

	public:
		JobScheduler();
		explicit JobScheduler(uint32_t max_num_threads);
		JobScheduler(const JobScheduler&) = delete;
		JobScheduler& operator=(const JobScheduler&) = delete;

		void initialize();
		void schedule(Job::Handle ch, bool b_thread_remain = false);

		uint32_t get_num_threads() const noexcept
		{
			return thread_pool.get_num_threads();
		}

		bool has_available_threads() const noexcept
		{
			return thread_pool.get_num_threads() > 0;
		}

	protected:
		ThreadPool thread_pool;	
		std::atomic_bool b_initialized{ false };

	public:
		static JobQueue* get_job_queue(uint32_t thread_index)
		{
			return thread_index > per_thread_queues.size() ? nullptr : &per_thread_queues[thread_index];
		}

	protected:
		static std::vector<JobQueue> per_thread_queues;		
	};

	struct suspend
	{
		struct awaiter : std::suspend_always
		{
			awaiter(bool thread_remain)
				: b_thread_remain(thread_remain)
			{ }

			void await_suspend(Job::Handle ch) const noexcept
			{
				JobScheduler::get()->schedule(ch, b_thread_remain);
			}

			bool b_thread_remain{ false };
		};

		suspend() = default;
		explicit suspend(bool thread_remain) noexcept
			: b_thread_remain(thread_remain)
		{ }

		auto operator co_await()
		{
			return awaiter{ b_thread_remain };
		}

		bool b_thread_remain{ false };
	};

	#define SUSPEND co_await suspend{ }
	#define SUSPEND_THREAD_REMAIN co_await suspend{ true }
}
