#ifndef JOBEXECUTER_H
#define JOBEXECUTER_H

#include <vector>
#include <mutex>
#include <thread>
#include <functional>

namespace jobs{

	template<typename RetType>
	class JobsExecuter;

	template<typename RetType>
	//the simple job
	struct Job {
		//Task - is a simple function or class or struct with 'operator()' overloaded
		//Task() return type must be non void
		std::function<RetType(void)> Task;
		//TaskEndCallBack - will be called, after task is done, with parameter returned by Task
		std::function<void(const RetType&)> TaskEndCallBack;
	private:
		bool JobIsDone;
		friend class JobsExecuter<RetType>;
		RetType result;
	};

	//Jobs - list of simple jobs
	template<typename RetType>
	struct Jobs{
		std::vector<Job<RetType>> Tasks;//list of jobs to be done
		std::function<void(void)> TasksEndCallBack;//will be called after all jobs is done
	private:
		std::vector<bool> JobsAreDone;
		friend class JobsExecuter<RetType>;
	};

	template<typename RetType>
	//performing single gob execution in paralel thread
	//
	class JobExecutor {
		//just creating working thread
		JobExecutor();

		//creating working thread and start executing newJob
		JobExecutor(const Job<RetType>& newJob);

		//waiting for job to be done
		//snd closing working thread
		~JobExecutor();

		//method is waiting until previous job will be done
		//and than start execute newJob (not waiting for in to be done)
		void AddJob(const Jobs<RetType>& newJobs);

		//true if the job not executed yet
		bool JobIsExecuting() const;

		//waiting until job will be done
		void WaitForJobDone() const;

		//true it job is done
		bool JobIsDone() const;

		//if job is not done jet, waiting until it be done 
		//than return result of the job 
		RetType JobResult() const;
	private:

	};

	template<typename RetType>
	//Performing executing of jobs
	//Using pool thread model to execute jobs concurrency
	class JobsExecuter
	{
	public:
		//just creating JobExecutors and wait for jobs
		JobsExecuter();
		
		//creating JobExecutors and allocates Jobs among them
		JobsExecuter(const Jobs<RetType>& newJobs);
		
		//waiting for all jobs to be done
		//snd closing alll threads
		~JobsExecuter();

		//method is waiting until previous jobs will be done
		//and than start execute newJobs (not waiting for them done)
		void AddJobs(const Jobs<RetType>& newJobs);

		//true if there is any job not executed yet
		bool JobsAreExecution() const;

		//waiting until all jobs will be done
		void WaitForJobsDone() const;

		//true it job # JobNum is done
		bool JobIsDone(const size_t& jobNum) const;

		//return current jobs count
		size_t JobsCount() const;

		//if job # jobNum is not done jet, waiting until it be done 
		//than return result of the job 
		RetType JobResult(const size_t& jobNum) const;

		//same as JobResult() but without range checking for jobNum
		RetType operator[] (const size_t& jobNum) const;

	private:
		Jobs _CurrentJobs;
		std::vector<JobExecutor> _WorkingThreads;
	};//class JobsExecuter

};//namespace jobs

jobs::JobsExecuter<int> a;

#endif //JOBEXECUTER_H