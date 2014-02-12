#include <string>
#include <mutex>
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include "../src/JobsExecuting.h"
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
		std::this_thread::sleep_for(std::chrono::seconds(1));
		m.unlock();
		return s.length();
	}
};

int main() {
	std::string s1[] = { "a", "bb", "ccc", "dddd" };
	using std::endl;
	using std::cout;
	jobs::Jobs<int> jobsTodo;
	jobs::JobExecuter<int> j1;
	for (auto &i : s1){
		jobsTodo.Tasks.push_back(jobs::Job<int>(myjob(i)));
		j1.AddJob(jobs::Job<int>(myjob(i)));
		int res = j1.JobResult();
		m.lock();
		cout << res << endl;
		m.unlock();
	}

	size_t jobsCount = 3;

	system("pause");
	return 0;
}