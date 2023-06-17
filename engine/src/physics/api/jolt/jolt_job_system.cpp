#include <physics/api/jolt/jolt_job_system.h>

namespace Sunset
{
	void JoltJobSystem::Init(uint32_t inMaxJobs, uint32_t inMaxBarriers)
	{
		JobSystemWithBarrier::Init(inMaxBarriers);

		// Init freelist of jobs
		mJobs.Init(inMaxJobs, inMaxJobs);
	}

	JoltJobSystem::JoltJobSystem(uint32_t inMaxJobs, uint32_t inMaxBarriers)
	{
		Init(inMaxJobs, inMaxBarriers);
	}
	
	JoltJobSystem::~JoltJobSystem()
	{
		// TODO: Ensure that there are no lingering jobs in the queue
	}

	JPH::JobHandle JoltJobSystem::CreateJob(const char* inJobName, JPH::ColorArg inColor, const JobFunction& inJobFunction, uint32_t inNumDependencies)
	{
		// Loop until we can get a job from the free list
		uint32_t index;
		for (;;)
		{
			index = mJobs.ConstructObject(inJobName, inColor, this, inJobFunction, inNumDependencies);
			if (index != AvailableJobs::cInvalidObjectIndex)
			{
				break;
			}
			assert(false && "No jobs available!");
			std::this_thread::sleep_for(std::chrono::microseconds(100));
		}
		Job* job = &mJobs.Get(index);

		// Construct handle to keep a reference, the job is queued below and may immediately complete
		JobHandle handle(job);

		// If there are no dependencies, queue the job now
		if (inNumDependencies == 0)
		{
			QueueJob(job);
		}

		// Return the handle
		return handle;
	}

	void JoltJobSystem::FreeJob(Job* inJob)
	{
		mJobs.DestructObject(inJob);
	}

	Sunset::Job JoltJobSystem::QueueSuspendableJob_Internal(Job* inJob)
	{
		// Sunset job scheduler will queue up this work on initial suspend (as a coroutine), before the call to Execute below.
		inJob->Execute();
		inJob->Release();
		co_return;
	}

	void JoltJobSystem::QueueJob(Job* inJob)
	{
		// If we have no worker threads, we can't queue the job either. We assume in this case that the job will be added to a barrier and that the barrier will execute the job when it's Wait() function is called.
		if (!JobScheduler::get()->has_available_threads())
		{
			return;
		}

		// Add reference to job because we're adding the job to the queue
		inJob->AddRef();

		// Queue the job
		QueueSuspendableJob_Internal(inJob);
	}

	void JoltJobSystem::QueueJobs(Job** inJobs, uint32_t inNumJobs)
	{
		assert(inNumJobs > 0);

		// If we have no worker threads, we can't queue the job either. We assume in this case that the job will be added to a barrier and that the barrier will execute the job when it's Wait() function is called.
		if (!JobScheduler::get()->has_available_threads())
		{
			return;
		}

		// Queue all jobs
		for (Job** job = inJobs, **job_end = inJobs + inNumJobs; job < job_end; ++job)
		{
			// Add reference to job because we're adding the job to the queue
			(*job)->AddRef();

			QueueSuspendableJob_Internal(*job);
		}
	}
}
