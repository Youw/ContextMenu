#include "MainLogic.h"

#include "JobsExecuting.h"
#include "CheckSumCalculator.h"
#include <thread>
#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <chrono>
#include <locale>

template<typename T>
//T must be STL container or array of std::wstring
static bool m_WriteInfoToLog(T&& Files, const std::wstring &LogFileName){
	using std::endl;
	std::wofstream LogFile;
	LogFile.imbue(std::locale(""));

	size_t filesCount = Files.size();

	std::vector<jobs::Job<CHECKSUM_DWORD>> jobsTodo;
	for (auto &f : Files){
		jobsTodo.push_back(jobs::Job<CHECKSUM_DWORD>(CheckSumCalculator(std::move(const_cast<std::wstring &>(f)))));
	}
	Files.clear();
	//start of the log
	{
		LogFile.open(LogFileName, std::ios::app);
		if (LogFile) {
			auto&& now = boost::posix_time::second_clock::local_time();
			LogFile << endl << "  <<<< Log created at: " << now << " >>>>" << endl << "Selected " << filesCount << " file(s)." << endl << endl;
			LogFile.close();
		}
		else {
			return false;
		}
	}

	jobs::JobsExecuter<CHECKSUM_DWORD> myJobs(std::move(jobsTodo));
	
	size_t jobsCount = myJobs.JobsCount();

	for (size_t i = 0; i < jobsCount; i++) {
		LogFile.open(LogFileName, std::ios::app);
		LogFile << "The checksum of \"" << myJobs.GetJob(i).Task.target<CheckSumCalculator>()->GetFileName() << "\" = " << myJobs[i] << endl;
		LogFile.close();
	}

	//end of the log
	{
		LogFile.open(LogFileName, std::ios::app);
		auto&& now = boost::posix_time::second_clock::local_time();
		LogFile << endl << "  <<<< End build log at: " << now << " >>>>" << endl;
		LogFile.close();
	}

	return true;
}

template<typename T>
//T must be STL container or array of std::wstring
bool WriteInfoToLog(T&& Files, const std::wstring &LogFileName, const bool async){
	if (async) {
		//i think it has a bag
		//TODO: fix it later
		std::thread working_thread(m_WriteInfoToLog, std::move(Files), std::move(LogFileName));
		working_thread.detach();
	}
	else {
		return m_WriteInfoToLog(Files, LogFileName);
	}
	return true;
}
