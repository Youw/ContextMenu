#ifndef JOBEXECUTER_H
#define JOBEXECUTER_H

#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>
#include <atomic>

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

		Job() : _JobIsDone(false) {};

		Job(
			const std::function<RetType(void)> &_Task, 
			const std::function<void(const RetType&)>& _TaskEndCallBack = nullptr) : 
				Task(_Task), 
				TaskEndCallBack(_TaskEndCallBack), 
				_JobIsDone(false) {};
		
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
	class JobExecuter {
	public:
		//just creating working thread
		JobExecuter(): _JobIsCome(false) {
			_WorkingThread = std::thread(JobExecuter<RetType>::JobThreadProcessor, this);
		};

		//can not be copied
		JobExecuter(const JobExecuter&) = delete;

		//move constructor
		JobExecuter(JobExecuter&& moveJob) { 
			*this = std::move(moveJob);
		};

		//can not be copied
		JobExecuter& operator=(const JobExecuter&) = delete;

		//move assigment operator
		JobExecuter& operator=(JobExecuter&& moveJob) {
			_JobIsCome = std::move(moveJob._JobIsCome);
			_mtxThreadJob = std::move(moveJob._mtxThreadJob);
			_CVmtxThreadJob = std::move(moveJob._CVmtxThreadJob);
			_mtxInterfaceJob = std::move(moveJob._mtxInterfaceJob);
			_CVmtxInterfaceJob = std::move(moveJob._CVmtxInterfaceJob);
			_CurrentJob = std::move(moveJob._CurrentJob);
			_WorkingThread = std::move(moveJob._WorkingThread);
		};

		//creating working thread and start executing newJob
		JobExecuter(Job<RetType>&& newJob): JobExecuter() {
			AddJob(std::move(newJob));
		};

		//waiting for job to be done
		//and closing working thread
		~JobExecuter() {
			WaitForJobDone();
			_ThreadShouldClose = true;
			_JobIsCome = true;
			_CVmtxThreadJob.notify_all();
			_WorkingThread.join();
		};

		//waiting until previous job will be done
		//and than start execute newJob (not waiting for it to be done)
		void AddJob(Jobs<RetType>&& newJobs);

		//waiting until job will be done
		void WaitForJobDone() const {
			if(!JobIsDone()) {
				//like wait until unlock
				_uLckThreadJob.lock();
				_uLckThreadJob.unlock();
			}
		};

		//true it job is done
		bool JobIsDone() const {
			return !bool(_uLckThreadJob);
		};

		//if job is not done jet, waiting until it be done 
		//than return result of the job 
		RetType JobResult() const{
			WaitForJobDone();
			return _CurrentJob._result;
		};
		
	private:	
		std::atomic_bool _JobIsCome;
		std::unique_lock<std::mutex> _uLckThreadJob;
		std::mutex _mtxThreadJob;
		std::condition_variable _CVmtxThreadJob;

		//std::mutex _mtxInterfaceJob;
		//std::condition_variable _CVmtxInterfaceJob;

		Job<RetType> _CurrentJob;
		std::atomic_bool _ThreadShouldClose;
		std::thread _WorkingThread;
		
		
		static void JobThreadProcessor(JobExecuter<RetType>* parent) {
			std::unique_lock<std::mutex> mtxLock(parent->_mtxThreadJob);
			while(true){
				while(!_JobIsCome) {
					_CVmtxThreadJob.wait(std::unique_lock(_mtxThreadJob));
				}
				if (_ThreadShouldClose) {
					return;
				}
			}
		}
		
		
	};

	template<typename RetType>
	//Performing executing of jobs
	//Using pool thread model to execute jobs concurrency
	class JobsExecuter {
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