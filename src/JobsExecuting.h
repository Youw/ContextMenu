#ifndef JOBEXECUTER_H
#define JOBEXECUTER_H

#include <vector>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <algorithm>
#include <deque>

namespace jobs{

	//declaration for Job
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
		std::function<void(RetType&)> TaskEndCallBack;

		Job() {
			_JobIsDone = false;
			_JobFailed = false;
		};

		Job(
			const std::function<RetType(void)> &_Task,
			const std::function<void(const RetType&)>& _TaskEndCallBack = nullptr) :
			Task(_Task),
			TaskEndCallBack(_TaskEndCallBack) {
			_JobIsDone = false;
			_JobFailed = false;
		};

		Job(
			std::function<RetType(void)> && _Task,
			std::function<void(const RetType&)>&& _TaskEndCallBack = nullptr) :
			Task(std::move(_Task)),
			TaskEndCallBack(std::move(_TaskEndCallBack)) {
			_JobIsDone = false;
			_JobFailed = false;
		};

		//can not be copied
		Job(const Job&) = delete;

		//move constructor
		Job(Job&& J) :
			Task(std::move(J.Task)),
			TaskEndCallBack(std::move(J.TaskEndCallBack)),
			_JobIsDone(J._JobIsDone),
			_JobFailed(J._JobFailed) {};

		//can not be copied
		Job& operator=(const Job&) = delete;

		//move assignment operator
		Job& operator=(Job&& J){
			Task = std::move(J.Task);
			TaskEndCallBack = std::move(J.TaskEndCallBack);
			_JobIsDone = J._JobIsDone;
			_JobFailed = J._JobFailed;
			return *this;
		}
		
		//true if job is done
		bool Done() const {
			return _JobIsDone;
		}

		//true if job has error occure
		bool Failed() const {
			return _JobFailed;
		}

	private:
		friend class JobExecuter<RetType>;
		friend class JobsExecuter<RetType>;
		//only JobExecuter will operate this fields
		std::atomic_bool  _JobIsDone;
		std::atomic_bool  _JobFailed;
		RetType _result;
	};

//	//Jobs - list of simple jobs
//	template<typename RetType>
//	struct Jobs{
//		std::vector< Job<RetType> > Tasks;//list of jobs to be done
//		std::function<void(void)> TasksEndCallBack;//will be called after all jobs are done
//
//		Jobs() {};
//
//		//can not be copied
//		Jobs(const std::vector< Job<RetType> > &_Tasks, const std::function<void(const RetType&)>& _TasksEndCallBack = nullptr) = delete;
//
//		Jobs(
//			std::vector< Job<RetType> >&& _Tasks,
//			std::function<void(const RetType&)>&& _TasksEndCallBack = nullptr) :
//			Tasks(std::move(_Tasks)),
//			TasksEndCallBack(std::move(_TasksEndCallBack)) {};
//
//		//can not be copied
//		Jobs(const Jobs&) = delete;
//
//		//move constructor
//		Jobs(Jobs&& J) :
//			Tasks(std::move(J.Tasks)),
//			TasksEndCallBack(std::move(J.TasksEndCallBack)),
//			_JobsAreDone(std::move(J._JobsAreDone)) {};
//
//		//can not be copied
//		Jobs& operator=(const Jobs&) = delete;
//
//		//move assignment operator
//		Jobs& operator=(Jobs&& J){
//			Tasks = std::move(J.Tasks);
//			TasksEndCallBack = std::move(J.TasksEndCallBack);
//			_JobsAreDone(J._JobsAreDone);
//			return *this;
//		}
//
//	private:
//		friend class JobsExecuter<RetType>;
//		//only JobsExecuter will operate this field
//
//		//std::vector<bool> JobsAreDone;
//		//there is some problems with vector<bool> in STL
//		std::vector<char> _JobsAreDone;
//
//	};

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

		//can not be moved
		JobExecuter(JobExecuter&& moveJob) = delete;
		//{
		//	_CVmtxThreadJob = std::move(moveJobExecuter._CVmtxThreadJob);
		//	_JobHasCome = std::move(moveJobExecuter._JobHasCome);
		//	_uLckThreadJob = std::move(moveJobExecuter._uLckThreadJob);
		//	_CurrentJob = std::move(moveJobExecuter._CurrentJob);
		//	_ThreadShouldClose = std::move(moveJobExecuter._ThreadShouldClose);
		//	_WorkingThread = std::move(moveJobExecuter._WorkingThread);
		//};

		//can not be copied
		JobExecuter& operator=(const JobExecuter&) = delete;

		//can not be moved
		JobExecuter& operator=(JobExecuter&& moveJobExecuter) = delete;

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
			_CurrentJob._JobFailed = false;
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
		bool JobDone() const {
			return _CurrentJob._JobIsDone;
		};

		//if job is not done jet, waiting until it be done 
		//than return result of the job 
		RetType JobResult() const{
			WaitForJobDone();
			return _CurrentJob._result;
		};
		
		bool JobFailed() const {
			WaitForJobDone();
			return _CurrentJob._JobFailed;
		}
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
				parent->_JobHasCome = false;
				parent->_synchronizeBOOL = true;
				if (parent->_ThreadShouldClose) {
					parent->_uLckThreadJob.release()->unlock();
					return;
				}
				try {
					parent->_CurrentJob._result = parent->_CurrentJob.Task();
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
					parent->_CurrentJob._JobFailed = true;
				}
			}
		}
	};

	template<typename RetType>
	//Performing executing of jobs
	//Using pool thread model to execute jobs concurrency
	class JobsExecuter {
	public:
		//just creating working threads that wait for jobs
		JobsExecuter(){
			_CurrentJobExecuting = 0;

			unsigned thCount = std::thread::hardware_concurrency();
			if (!thCount) thCount = 3;//why not =)

			std::atomic_bool temp_var;
			temp_var = false;
			_ThreadShouldClose.resize(thCount, temp_var);
			_mtxThreadJob.resize(thCount);
			_uLckThreadJob.resize(thCount);
			_synchronizeBOOL.resize(thCount, temp_var);
			//threads, where Jobs will be executed
			for (unsigned i = 0; i < thCount; i++){
				_WorkingThreads.push_back(std::thread(JobsExecuter<RetType>::JobThreadProcessor,this,i));
			}
			while (!std::all_of(_synchronizeBOOL.begin(), _synchronizeBOOL.end(),
				[&](std::atomic_bool&i){ return i; })) {
				std::this_thread::yield();
			}
		};
		
		//can not be copied
		JobsExecuter(const JobsExecuter&) = delete;

		//can not be copied
		JobsExecuter& operator=(const JobsExecuter&) = delete;

		//can not be moved
//		JobsExecuter& (JobsExecuter<RetType>&& ) = delete;

		//can not be moved
		JobsExecuter& operator=(JobsExecuter&&) = delete;

		//improve perfomance - do not copy objects
//		JobsExecuter(const Jobs<RetType>&) = delete;

		//creating JobExecutors and allocates Jobs among them
		JobsExecuter(std::vector< Job<RetType> >&& newJobs) :JobsExecuter() {
			AddJobs(std::move(newJobs));
		};
		
		//waiting for all jobs to be done
		//and closing all threads
		~JobsExecuter() {
			WaitForAllJobsDone();
			for (auto& i : _ThreadShouldClose) { i = true; }
			this->_CurrentJobExecuting = 0;
			_CVmtxThreadJob.notify_all();
			for (auto &i : _WorkingThreads) { i.join(); };
		};

		//method is waiting until previous jobs will be done
		//and than start execute newJobs (not waiting for them to be done)
		void AddJobs(std::vector< Job<RetType> >&& newJobs) {
			WaitForAllJobsDone();
			std::lock_guard<std::mutex> lck(_mtxCurrentJobs);
			_CurrentJobs = std::move(newJobs);
			_CurrentJobExecuting  = 0;
			_JobsThreadNum.resize(_CurrentJobs.size());
			for (auto& i : _JobsThreadNum) {i = -1;}
			_mtxJobs.resize(_CurrentJobs.size());
			_uLckMtxJobs.resize(_CurrentJobs.size());
			_CVmtxThreadJob.notify_all();
		};

		//can not be copied
		void AddJobs(const std::vector< Job<RetType> >& newJobs) = delete;

		//true if there is any job not executed yet
		bool JobsAreExecution() const {
			return std::any_of(_CurrentJobs.begin(), _CurrentJobs.end(), [&](const Job<RetType>& i) {
				return !(i.Done());
			});
		};

		bool AllDone() const {
			std::lock_guard<std::mutex> lck(_mtxCurrentJobs);
			return  _CurrentJobExecuting<_CurrentJobs.size();
		}

		//waiting until all jobs will be done
		void WaitForAllJobsDone() const {
			if (!std::all_of(_CurrentJobs.begin(), _CurrentJobs.end(), [&](const Job<RetType>& i) {
				return i.Done();
			})) {
				if (_uLckMtxAllJobs.mutex() == nullptr) 
					_uLckMtxAllJobs = std::unique_lock<std::mutex>(_mtxAllJobs);
				while (!AllDone())
					_CVmtxJobs.wait(_uLckMtxAllJobs);
				try {
					std::mutex *tmp = _uLckMtxAllJobs.release();
					if (tmp)
						tmp->unlock();
				}
				catch (...) {}
			}
		};

		//waiting until specific job will be done
		void WaitForJobDone(size_t jobNum) const {
			if (!JobDone(jobNum)) {
				if (_uLckMtxJobs[jobNum].mutex() == nullptr)
					_uLckMtxJobs[jobNum] = std::unique_lock<std::mutex>(_mtxJobs[jobNum]);
				_CVmtxJobs.wait(_uLckMtxJobs[jobNum], [&]() {
					return _CurrentJobs[jobNum].Done();
				});
				try {
					std::mutex *tmp = _uLckMtxJobs[jobNum].release();
					if (tmp)
						tmp->unlock();
				}
				catch (...) {}
			}
		};

		//true it job # JobNum is done
		bool JobDone(size_t jobNum) const {
			std::lock_guard<std::mutex> lck(_mtxCurrentJobs);
			if ((jobNum >= 0) || (jobNum < _CurrentJobs.size())) {
				return _CurrentJobs[jobNum]._JobIsDone;
			}
			throw std::out_of_range("Job index out of range.");
		};

		//return current jobs count
		size_t JobsCount() const {
			std::lock_guard<std::mutex> lck(_mtxCurrentJobs);
			return _CurrentJobs.size();
		};

		//if job # jobNum is not done jet, waiting until it be done
		//than return result of the job 
		RetType JobResult(size_t jobNum) const{
			std::lock_guard<std::mutex> lck(_mtxCurrentJobs);
			if ((jobNum >= 0) || (jobNum < _CurrentJobs.size())) {
				return operator[](jobNum);
			}
			throw std::out_of_range("Job index out of range.");
		};


		//same as JobResult() but without range checking for jobNum
		RetType operator[] (size_t jobNum) const {
			WaitForJobDone(jobNum);
			if (_CurrentJobs[jobNum]._result < 0) {
				WaitForJobDone(jobNum);
			}
			return _CurrentJobs[jobNum]._result;
		};

		//obviously
		const Job<RetType>& GetJob(size_t jobNum) const {
			std::lock_guard<std::mutex> lck(_mtxCurrentJobs);
			if ((jobNum >= 0) || (jobNum < _CurrentJobs.size())) {
				return _CurrentJobs[jobNum];
			}
			throw std::out_of_range("Job index out of range.");
		};

	private:
		//to synchronize pooling out jobs
		std::condition_variable _CVmtxThreadJob;
		//for _CVmtxThreadJob
		mutable std::vector <std::unique_lock<std::mutex> > _uLckThreadJob;
		//for _uLckThreadJob
		std::deque<std::mutex> _mtxThreadJob;

		//Jobs staff
		std::vector< Job<RetType> > _CurrentJobs;
		mutable std::mutex _mtxCurrentJobs;
		std::atomic<size_t> _CurrentJobExecuting;
		//the number of thread in _WorkingThreads there Job is executing now
		std::vector<int> _JobsThreadNum;
		
		//staff for call back when job is done
		mutable std::condition_variable _CVmtxJobs;
		mutable std::deque<std::mutex> _mtxJobs;
		mutable std::vector< std::unique_lock<std::mutex> > _uLckMtxJobs;
		mutable std::mutex _mtxAllJobs;
		mutable std::unique_lock<std::mutex> _uLckMtxAllJobs;

		//true when thread's death is coming
		std::vector<std::atomic_bool> _ThreadShouldClose;

		//threads, where Jobs will be executed
		std::vector<std::thread> _WorkingThreads;

		//some synchronize staff
		std::vector<std::atomic_bool> _synchronizeBOOL;

		static void JobThreadProcessor(JobsExecuter<RetType>* parent, int _ThreadNum) {
			parent->_uLckThreadJob[_ThreadNum] = std::unique_lock<std::mutex>(parent->_mtxThreadJob[_ThreadNum]);
			parent->_synchronizeBOOL[_ThreadNum] = true;
			//and now thread started

			size_t ThisThreadJob;
			while (true) {	
				parent->_CVmtxThreadJob.wait(parent->_uLckThreadJob[_ThreadNum], [&](){
					std::lock_guard<std::mutex> lck(parent->_mtxCurrentJobs);
					bool tmp = parent->_CurrentJobExecuting < parent->_CurrentJobs.size() ;
					if (tmp) {
						ThisThreadJob = parent->_CurrentJobExecuting;
						parent->_JobsThreadNum[ThisThreadJob] = _ThreadNum;
						if (parent->_ThreadShouldClose[_ThreadNum]) {
							return true;
						}
						(parent->_CurrentJobExecuting)++;
					}
					if (parent->_ThreadShouldClose[_ThreadNum]) {
						return true;
					}
					return tmp;
				});
//				parent->_synchronizeBOOL[_ThreadNum] = true;
				if (parent->_ThreadShouldClose[_ThreadNum]) {
					parent->_uLckThreadJob[_ThreadNum].release()->unlock();
					return;
				}
				Job<RetType>& theJob = parent->_CurrentJobs[ThisThreadJob];
				try {
					theJob._result = theJob.Task();
					try {
						if (theJob.TaskEndCallBack) {
							theJob.TaskEndCallBack(theJob._result);
						}
					}
					catch (...) {

					}
					theJob._JobIsDone = true;
				}
				catch (...) {
					theJob._JobIsDone = false;
					theJob._JobFailed = true;
				}
				parent->_JobsThreadNum[ThisThreadJob] = -1;
				parent->_CVmtxJobs.notify_all();
			}
		}

	};//class JobsExecuter

};//namespace jobs

#endif //JOBEXECUTER_H