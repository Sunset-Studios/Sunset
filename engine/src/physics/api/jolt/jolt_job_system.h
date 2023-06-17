#pragma once

#include <minimal.h>
#include <job_system/job_scheduler.h>

#include <Jolt/Jolt.h>
#include <Jolt/Core/JobSystemWithBarrier.h>
#include <Jolt/Core/FixedSizeFreeList.h>
#include <Jolt/Core/Semaphore.h>

namespace Sunset
{
	class JoltJobSystem final : public JPH::JobSystemWithBarrier
	{
	public:
		JPH_OVERRIDE_NEW_DELETE

		/// Creates a job system for jolt related work.
		/// @see JoltJobSystem::Init
		JoltJobSystem() = default;
		JoltJobSystem(uint32_t inMaxJobs, uint32_t inMaxBarriers);
		virtual	~JoltJobSystem() override;

		/// Initialize the jolt job system
		/// @param inMaxJobs Max number of jobs that can be allocated at any time
		/// @param inMaxBarriers Max number of barriers that can be allocated at any time
		void Init(uint32_t inMaxJobs, uint32_t inMaxBarriers);

		// See JobSystem
		virtual int	GetMaxConcurrency() const override { return JobScheduler::get()->get_num_threads(); }
		virtual JobHandle CreateJob(const char* inName, JPH::ColorArg inColor, const JobFunction& inJobFunction, uint32_t inNumDependencies = 0) override;

	protected:
		// See JobSystem
		virtual void QueueJob(Job* inJob) override;
		virtual void QueueJobs(Job** inJobs, uint32_t inNumJobs) override;
		virtual void FreeJob(Job* inJob) override;

	private:
		/// Internal helper function to queue a job
		inline Sunset::Job QueueSuspendableJob_Internal(Job* inJob);

		/// Array of jobs (fixed size)
		using AvailableJobs = JPH::FixedSizeFreeList<Job>;
		AvailableJobs mJobs;
	};
}
