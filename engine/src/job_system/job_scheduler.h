#pragma once

#include <minimal.h>
#include <job_system/thread_pool.h>
#include <utility/pattern/singleton.h>
#include <atomic_queue/atomic_queue.h>

#include <coroutine>

namespace Sunset
{
	constexpr uint32_t MAX_JOB_QUEUE_SIZE = 8192;
	constexpr uint32_t MAX_SCHEDULER_THREADS = 32;

	template<int32_t ThreadIndex = -1>
	struct ThreadedJob
	{
		struct promise_type
		{
			ThreadedJob get_return_object(); 
			std::suspend_always initial_suspend() noexcept;
			std::suspend_never final_suspend() noexcept;
			void unhandled_exception() {}
			void return_void() {}

			std::coroutine_handle<promise_type> handle;
			int32_t thread_index{ ThreadIndex };
		};

		ThreadedJob() = default;
		ThreadedJob(const std::coroutine_handle<promise_type>& handle);

		bool is_done() const;

		std::coroutine_handle<promise_type> handle;

		using Handle = std::coroutine_handle<promise_type>;
	};

	struct Job
	{
		struct promise_type
		{
			Job get_return_object() { return { handle }; }
			std::suspend_always initial_suspend() noexcept { return {}; }
			std::suspend_never final_suspend() noexcept { return {}; }
			void unhandled_exception() {}
			void return_void() {}

			std::coroutine_handle<> handle;	
		};

		Job() = default;
		Job(const std::coroutine_handle<>& handle);

		bool is_done() const;

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

		void set_job_done_state(void* address, bool b_done_state)
		{
			job_done_states[address] = b_done_state;
		}

		bool get_job_done_state(void* address)
		{
			return job_done_states[address]; 
		}

	protected:
		ThreadPool thread_pool;	
		phmap::parallel_flat_hash_map<void*, bool> job_done_states;
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
	inline ThreadedJob<ThreadIndex>::ThreadedJob(const std::coroutine_handle<promise_type>& handle)
		: handle(handle)
	{
		JobScheduler::get()->set_job_done_state(handle.address(), false);
	}

	template<int32_t ThreadIndex>
	inline bool ThreadedJob<ThreadIndex>::is_done() const
	{
		return JobScheduler::get()->get_job_done_state(handle.address());
	}

	template<int32_t ThreadIndex>
	std::suspend_always ThreadedJob<ThreadIndex>::promise_type::initial_suspend() noexcept
	{
		Job job{ ThreadedJob<ThreadIndex>::Handle::from_promise(*this) };
		thread_index = JobScheduler::get()->schedule(job.handle, thread_index, false, true);
		return {};
	}

	template<int32_t ThreadIndex>
	ThreadedJob<ThreadIndex> ThreadedJob<ThreadIndex>::promise_type::get_return_object()
	{
		handle = ThreadedJob<ThreadIndex>::Handle::from_promise(*this);
		return { handle };
	}

	template<int32_t ThreadIndex>
	std::suspend_never ThreadedJob<ThreadIndex>::promise_type::final_suspend() noexcept
	{
		JobScheduler::get()->set_job_done_state(handle.address(), true);
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

	template<typename T>
	class JobBatcher
	{
		public:
			JobBatcher(size_t size)
				: pending_jobs(size)
			{ }
			JobBatcher(std::initializer_list<T>&& jobs)
				: pending_jobs(std::move(jobs))
			{ }
			~JobBatcher() = default;

			void add(T job, int32_t index = -1);
			void wait_on_all();

		protected:
			std::vector<T> pending_jobs;	
	};

	template<typename T>
	inline void JobBatcher<T>::add(T job, int32_t index)
	{
		if (index >= 0)
		{
			pending_jobs[index] = std::move(job);
		}
		else
		{
			pending_jobs.emplace_back(std::move(job));
		}
	}

	template<typename T>
	inline void JobBatcher<T>::wait_on_all()
	{
		for (T& job : pending_jobs)
		{
			while (!job.is_done())
			{
				std::this_thread::sleep_for(std::chrono::nanoseconds(1));
			}
		}
	}

	void parallel_for(uint32_t iterations, std::function<void(uint32_t)> op);
}
