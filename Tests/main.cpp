#include <string>
#include <mutex>
#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>
#include "../src/JobsExecuting.h"
std::mutex m;
template<typename T>
static void out(const T& s) {
//	m.lock();
	std::cout << s << std::endl;
//	m.unlock();
}

class myjob {
public:
	std::string s;
	
	myjob(const std::string& s) {
		this->s = s;
	}
	int operator() () {
//	std::this_thread::sleep_for(std::chrono::seconds(1));
		m.lock();

//		std::this_thread::sleep_for(std::chrono::seconds(1));
		std::cout << s << std::endl;
		
		
		m.unlock();
		return s.length();
	}
};

#include<Windows.h>

int main() {
	std::wstring a = L"ïÐèÂ³Ò hELlo";
	std::wstring b = L"ïðèâ³ò hello";

	std::locale l("");
	std::cout << l.name() << std::endl;
	std::string s1[] = {"a", "bb"  , "ccc", "dddd", "eeeee", "ffffff" };
	using std::endl;
	using std::cout;
	std::vector<jobs::Job<int>> jobsTodo;
	for (auto &i : s1){
		jobsTodo.push_back(jobs::Job<int>(myjob(i)));
//		j1.AddJob(jobs::Job<int>(myjob(i)));
//		int res = j1.JobResult();
//		m.lock();
//		cout << res << endl;
//		m.unlock();
	}
	jobs::JobsExecuter<int> J(std::move(jobsTodo));

	//std::this_thread::sleep_for(std::chrono::seconds(4));

	for (unsigned i = 0; i < J.JobsCount(); i++){
		out(J[i]);
	}

	system("pause");
	return 0;
}