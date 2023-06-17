#include <gtest/gtest.h>
#include <job_system/thread_pool.h>
#include <job_system/job_scheduler.h>

#include <syncstream>

using namespace Sunset;

class SchedulerTestMethods
{
public:
	static Job test_func_1()
	{
		std::osyncstream synched_cout(std::cout);

		synched_cout << "test_func_1: Hello from thread: " << std::this_thread::get_id() << std::endl;

		SUSPEND;

		synched_cout << "test_func_1: Hello again from thread: " << std::this_thread::get_id() << std::endl;
	}
};

TEST(SunsetTests, ThreadPool_RunUniqueThreads)
{
	ThreadPool thread_pool;
	const uint32_t max_threads = thread_pool.get_num_threads();
	for (uint32_t i = 0; i < max_threads; ++i)
	{
		thread_pool.run_on_thread([](std::stop_token st)
		{
			std::osyncstream synched_cout(std::cout);
			synched_cout << "In thread: " << std::this_thread::get_id() << std::endl;
		}, i);
	}
}

TEST(SunsetTests, JobScheduler_ScheduleJobs)
{
	SchedulerTestMethods::test_func_1();
	SchedulerTestMethods::test_func_1();
}