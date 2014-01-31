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
	//Performing executing of jobs
	//Using pool thread to execute jobs concurrency
	class JobsExecuter
	{
	public:
		//just creating working threads, suspend them and wait for jobs
		JobsExecuter();
		
		//creating working  threads and start execute new jobs
		JobsExecuter(const Jobs<RetType>& newJobs);
		
		//waiting for all jobs to be done
		//snd closing alll threads
		~JobsExecuter();

		//method is waiting until previous jobs will be done
		//and than start execute new jobs and return
		void AddJobs(const Jobs<RetType>& newJobs);

		//true if there is any job not executed yet
		bool JobsAreExecution() const;

		//waiting until all jobs will be done
		void WaitForJobsDone() const;

		//true it job ¹ JobNum is done
		bool JobIsDone(const size_t& jobNum) const;

		//return current jobs count
		size_t JobsCount() const;

		//if job ¹ jobNum is not done jet, waiting until it be done 
		//than return result of the job 
		RetType JobResult(const size_t& jobNum) const;

		//same as JobResult() but without range checking for jobNum
		RetType operator[] (const size_t& jobNum) const;

	private:
		Jobs CurrentJobs;
		std::vector<std::thread> WorkingThreads;
	};//class JobsExecuter

};//namespace jobs

jobs::JobsExecuter<int> a;
#endif //JOBEXECUTER_H