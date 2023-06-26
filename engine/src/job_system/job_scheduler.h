#pragma once

#include <minimal.h>
#include <job_system/thread_pool.h>
#include <utility/pattern/singleton.h>

#include <atomic_queue/atomic_queue.h>

#include <coroutine>

namespace Sunset
{
	constexpr uint32_t MAX_JOB_QUEUE_SIZE = 4096;
	constexpr uint32_t MAX_SCHEDULER_THREADS = 32;

	template<int32_t ThreadIndex = -1>
	struct ThreadedJob
	{
		struct promise_type
		{
			ThreadedJob get_return_object() { return {}; }
			std::suspend_always initial_suspend() noexcept;
			std::suspend_never final_suspend() noexcept { return {}; }
			void unhandled_exception() {}
			void return_void() {}

			int32_t thread_index{ ThreadIndex };
		};

		using Handle = std::coroutine_handle<promise_type>;
	};

	struct Job
	{
		Job(const std::coroutine_handle<>& handle)
			: handle(handle)
		{ }

		struct promise_type
		{
			Job get_return_object() { return { handle }; }
			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_never final_suspend() noexcept { return {}; }
			void unhandled_exception() {}
			void return_void() {}

			std::coroutine_handle<> handle;	
		};

		std::coroutine_handle<> handle;

		using Handle = std::coroutine_handle<>;
	};


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
		int32_t schedule(Job::Handle ch, uint32_t thread_index = -1, bool b_thread_remain = false, bool b_initial_run = false);

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

	template<int32_t ThreadIndex>
	std::suspend_always ThreadedJob<ThreadIndex>::promise_type::initial_suspend() noexcept
	{
		Job job{ ThreadedJob<ThreadIndex>::Handle::from_promise(*this) };
		thread_index = JobScheduler::get()->schedule(job.handle, thread_index, false, true);
		return {};
	}

	struct suspend
	{
		struct awaiter : std::suspend_always
		{
			awaiter(bool thread_remain)
				: b_thread_remain(thread_remain)
			{ }

			void await_suspend(Job::Handle ch) const noexcept
			{
				auto _ = JobScheduler::get()->schedule(ch, b_thread_remain);
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
