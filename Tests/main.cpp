
#include "../src/JobsExecuting.h"
#include <string>
#include <mutex>
#include <iostream>
#include <thread>
#include <atomic>

std::mutex m;

class myjob {
public:
	std::string s;
	
	myjob(const std::string& s) {
		this->s = s;
	}
	int operator() () {
		m.lock();
		std::cout << s << std::endl;
		m.unlock();
		return s.length();
	}
};

std::atomic<bool> isready(false);
std::mutex m1;
std::condition_variable cv;

void func1() {
	std::unique_lock<std::mutex> lck(m1);
	while(!isready) {
		cv.wait(lck);
	}
	static int i;
	std::cout << "func1() " << i++ << std::endl;
 }

 #include <chrono>
int main() {
	isready = true;
	cv.notify_all();
	std::thread a(func1);
	std::thread b(func1);
	//std::this_thread::sleep_for(std::chrono::seconds(2));
	a.join();
	b.join();
	
	std::string s[] = {"1","22", "3333"};
	using std::endl;
	using std::cout;
	jobs::Jobs<int> jobsTodo;
	for (auto &i : s){
		jobsTodo.Tasks.push_back(jobs::Job<int>(myjob(i)));
	}
	
	jobs::JobExecuter<int> j1(myjob(s[0]));
	
//	jobs::JobsExecuter<CHECKSUM_DWORD> myJobs(jobsTodo);

	size_t jobsCount = 3;

	for (size_t i = 0; i < jobsCount; i++) {
		m.lock();
		cout << "" << endl;
		m.unlock();
	}

	return 0;
}