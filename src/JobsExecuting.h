#ifndef JOBEXECUTER_H
#define JOBEXECUTER_H

#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>

namespace jobs{

	//declaration for Log and Lobs
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

		Job() : JobIsDone(false) {};

		Job(
			const std::function<RetType(void)> &_Task, 
			const std::function<void(const RetType&)>& _TaskEndCallBack = nullptr) : 
				Task(_Task), 
				TaskEndCallBack(_TaskEndCallBack), 
				JobIsDone(false) {};
		
		Job(
			std::function<RetType(void)> && _Task,
			std::function<void(const RetType&)>&& _TaskEndCallBack = nullptr) :
				Task(std::move(_Task)), 
				TaskEndCallBack(std::move(_TaskEndCallBack)), 
				_JobIsDone(false) {};
		
		//can not be copied
		Job(const Job&) = delete;

		//move constructor
		Job(Job&& J) : 
			Task(std::move(J.Task)),
			TaskEndCallBack(std::move(J.TaskEndCallBack)),
			_JobIsDone(J._JobIsDone) {};
		
		//can not be copied
		Job& operator=(const Job&) = delete;
		
		//move assignment operator
		Job& operator=(Job&& J){
			Task = std::move(J.Task);
			TaskEndCallBack = std::move(J.TaskEndCallBack);
			_JobIsDone(J._JobIsDone);
			return *this;
		}
	private:
		friend class JobsExecuter<RetType>;
		//only JobsExecuter will operate this fields
		bool _JobIsDone;
		RetType _result;
	};

	//Jobs - list of simple jobs
	template<typename RetType>
	struct Jobs{
		std::vector< Job<RetType> > Tasks;//list of jobs to be done
		std::function<void(void)> TasksEndCallBack;//will be called after all jobs are done
		
		Jobs() {};

		//can not be copied
		Jobs(const std::vector< Job<RetType> > &_Tasks, const std::function<void(const RetType&)>& _TasksEndCallBack = nullptr) = delete;
		
		Jobs(
			std::vector< Job<RetType> >&& _Tasks, 
			std::function<void(const RetType&)>&& _TasksEndCallBack = nullptr) :
				Tasks(std::move(_Tasks)), 
				TasksEndCallBack(std::move(_TasksEndCallBack)) {};
		
		//can not be copied
		Jobs(const Jobs&) = delete;

		//move constructor
		Jobs(Jobs&& J) : 
			Tasks(std::move(J.Tasks)),
			TasksEndCallBack(std::move(J.TasksEndCallBack)),
			_JobsAreDone(std::move(J._JobsAreDone)) {};
		
		//can not be copied
		Jobs& operator=(const Jobs&) = delete;
		
		//move assignment operator
		Jobs& operator=(Jobs&& J){
			Tasks = std::move(J.Tasks);
			TasksEndCallBack = std::move(J.TasksEndCallBack);
			_JobsAreDone(J._JobsAreDone);
			return *this;
		}
		
	private:
		friend class JobsExecuter<RetType>;
		//only JobsExecuter will operate this field
		
		//std::vector<bool> JobsAreDone;
		//there is some problems with vector<bool> in STL
		std::vector<char> _JobsAreDone;
		
	};

	template<typename RetType>
	//performing single gob execution in paralel thread
	//thread's lifetime is the same as class JobExecuter's object lifetime
	class JobExecuter {
		//just creating working thread
		JobExecuter();

		//can not be copied
		JobExecuter(const JobExecuter&) = delete;

		//move constructor
		JobExecuter(JobExecuter&&);

		//can not be copied
		JobExecuter& operator=(const JobExecuter&) = delete;

		//move assigment operator
		JobExecuter& operator=(JobExecuter&&);

		//creating working thread and start executing newJob
		JobExecuter(Job<RetType>&& newJob);

		//waiting for job to be done
		//and closing working thread
		~JobExecuter();

		//waiting until previous job will be done
		//and than start execute newJob (not waiting for it to be done)
		void AddJob(Jobs<RetType>&& newJobs);

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
		// the function running in other thread
		static void _ThreadProcessor(JobExecuter& _this);

		std::mutex _mtxThreadJob;
		std::condition_variable _CVmtxThreadJob;

		std::mutex _mtxInterfaceJob;
		std::condition_variable _CVmtxInterfaceJob;

		Job<RetType> _CurrentJob;
		std::thread _WorkingThread;
		
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

		const Job<RetType>& GetJob(size_t pos) const;

	private:
		Jobs<RetType> _CurrentJobs;
		std::vector< JobExecuter<RetType> > _WorkingThreads;
	};//class JobsExecuter

};//namespace jobs


#endif //JOBEXECUTER_H
