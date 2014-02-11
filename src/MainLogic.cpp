#include "MainLogic.h"

#include "JobsExecuting.h"
#include "CheckSumCalculator.h"
#include <thread>
#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include <chrono>

static bool m_WriteInfoToLog(std::set<std::wstring> &Files, const std::wstring &LogFileName){
	using std::endl;
	jobs::Jobs<CHECKSUM_DWORD> jobsTodo;
	for (auto &f : Files){
		jobsTodo.Tasks.push_back(jobs::Job<CHECKSUM_DWORD>(CheckSumCalculator(std::move(const_cast<std::wstring &>(f)))));
	}
	Files.clear();
	//start of the log
	{
		std::wofstream LogFile(LogFileName, std::ios::app);
		if (LogFile) {
			auto&& now = boost::posix_time::second_clock::local_time();
			LogFile << endl << "  <<<< Log created at: " << now << " >>>>" << endl << endl;
		}
		else {
			return false;
		}
	}



	//end of the log
	{
		std::wofstream LogFile(LogFileName, std::ios::app);
		if (LogFile) {
			auto&& now = boost::posix_time::second_clock::local_time();
			LogFile << endl << "  <<<< End build log at: " << now << " >>>>" << endl;

		}
		else {
			return false;
		}
	}

	return true;
}


bool WriteInfoToLog(std::set<std::wstring> &Files, const std::wstring &LogFileName, const bool async){
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
