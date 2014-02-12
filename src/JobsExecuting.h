#ifndef JOBEXECUTER_H
#define JOBEXECUTER_H

#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>
#include <atomic>

namespace jobs{

	//declaration for Job and Jobs
	template<typename RetType>
	class JobExecuter;
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

		Job() {
			_JobIsDone = false;
			_JobFaild = false;
		};

		Job(
			const std::function<RetType(void)> &_Task,
			const std::function<void(const RetType&)>& _TaskEndCallBack = nullptr) :
			Task(_Task),
			TaskEndCallBack(_TaskEndCallBack) {
			_JobIsDone = false;
			_JobFaild = false;
		};

		Job(
			std::function<RetType(void)> && _Task,
			std::function<void(const RetType&)>&& _TaskEndCallBack = nullptr) :
			Task(std::move(_Task)),
			TaskEndCallBack(std::move(_TaskEndCallBack)) {
			_JobIsDone = false;
			_JobFaild = false;
		};

		//can not be copied
		Job(const Job&) = delete;

		//move constructor
		Job(Job&& J) :
			Task(std::move(J.Task)),
			TaskEndCallBack(std::move(J.TaskEndCallBack)),
			_JobIsDone(J._JobIsDone),
			_JobFaild(J._JobFaild) {};

		//can not be copied
		Job& operator=(const Job&) = delete;

		//move assignment operator
		Job& operator=(Job&& J){
			Task = std::move(J.Task);
			TaskEndCallBack = std::move(J.TaskEndCallBack);
			_JobIsDone = J._JobIsDone;
			_JobFaild = J._JobFaild;
			return *this;
		}
	private:
		friend class JobExecuter<RetType>;
		//only JobExecuter will operate this fields
		std::atomic_bool  _JobIsDone;
		std::atomic_bool  _JobFaild;
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

}; //namespace jobs

	extern std::mutex m;

namespace jobs {
	template <typename T>
	void out(T s) {
		std::lock_guard<std::mutex> tmp(m);
		std::cout << s << std::endl;
	}

	template<typename RetType>
	class JobExecuter {
	public:
		//just creating working thread
		JobExecuter() : _mtxThreadJob() {
			_synchronizeBOOL = false;
			_ThreadShouldClose = false;
			_JobHasCome = false;
			_WorkingThread = std::thread(JobExecuter<RetType>::JobThreadProcessor, this);
			while (!_synchronizeBOOL) {
				std::this_thread::yield();
			}
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
		JobExecuter& operator=(JobExecuter&& moveJobExecuter) {
			_CVmtxThreadJob = std::move(moveJobExecuter._CVmtxThreadJob);
			_JobHasCome = std::move(moveJobExecuter._JobHasCome);
			_uLckThreadJob = std::move(moveJobExecuter._uLckThreadJob);
			_CurrentJob = std::move(moveJobExecuter._CurrentJob);
			_ThreadShouldClose = std::move(moveJobExecuter._ThreadShouldClose);
			_WorkingThread = std::move(moveJobExecuter._WorkingThread);
		};

		//creating working thread and 
		//wait for starting of executing newJob
		JobExecuter(Job<RetType>&& newJob): JobExecuter() {
			AddJob(std::move(newJob));
		};

		//waiting for job to be done
		//and closing working thread
		~JobExecuter() {
			WaitForJobDone();
			_ThreadShouldClose = true;
			_JobHasCome = true;
			_CVmtxThreadJob.notify_all();
			_WorkingThread.join();
		};

		//waiting until previous job will be done
		//and than start execute newJob (not waiting for it to be done)
		void AddJob(Job<RetType>&& newJob){
			WaitForJobDone();
			_CurrentJob = std::move(newJob);
			_CurrentJob._JobIsDone = false;

			_synchronizeBOOL = false;
			_JobHasCome = true;
			_CVmtxThreadJob.notify_all();
			while (!_synchronizeBOOL) {
				std::this_thread::yield();
			}
		};

		//waiting until job will be done
		void WaitForJobDone() const {
			if (!JobIsDone()) {
				//like wait until unlock
				_uLckThreadJob.mutex()->lock();
				_uLckThreadJob.mutex()->unlock();
			}
		};

		//true it job is done
		bool JobIsDone() const {
			return _CurrentJob._JobIsDone;
		};

		//if job is not done jet, waiting until it be done 
		//than return result of the job 
		RetType JobResult() const{
			WaitForJobDone();
			return _CurrentJob._result;
		};
		

	private:	
		//to synchronize thread and other functions
		std::condition_variable _CVmtxThreadJob;

		//obviously
		std::atomic_bool _JobHasCome;

		//for std::condition_variable _CVmtxThreadJob
		mutable std::unique_lock<std::mutex> _uLckThreadJob;
		
		//for std::unique_lock<std::mutex> _uLckThreadJob
		std::mutex _mtxThreadJob;

		//the essence =)
		Job<RetType> _CurrentJob;

		//true when object's death is coming...
		std::atomic_bool _ThreadShouldClose;

		//thread, where Job will be executed
		std::thread _WorkingThread;
		
		//some synchronize staff
		std::atomic_bool _synchronizeBOOL;

		static void JobThreadProcessor(JobExecuter<RetType>* parent){
			parent->_uLckThreadJob = std::unique_lock<std::mutex>(parent->_mtxThreadJob);
			parent->_synchronizeBOOL = true;
			//and now thread started
			while(true) {
				parent->_CVmtxThreadJob.wait(parent->_uLckThreadJob, [&](){ return parent->_JobHasCome; });
				parent->_synchronizeBOOL = true;
				if (parent->_ThreadShouldClose) {
					parent->_uLckThreadJob.release()->unlock();
					return;
				}
				try {
					parent->_CurrentJob._result = parent->_CurrentJob.Task();
					parent->_JobHasCome = false;
					try {
						if (parent->_CurrentJob.TaskEndCallBack) {
							parent->_CurrentJob.TaskEndCallBack(parent->_CurrentJob._result);
						}
					}
					catch (...) {

					}
					parent->_CurrentJob._JobIsDone = true;
				}
				catch (...) {
					parent->_CurrentJob._JobIsDone = false;
					parent->_CurrentJob._JobFaild = true;
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