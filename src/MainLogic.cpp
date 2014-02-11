#include "MainLogic.h"

#include "JobsExecuting.h"
#include "CheckSumCalculator.h"
#include <thread>
#include <boost/filesystem.hpp>

static bool m_WriteInfoToLog(std::set<std::wstring> &Files, const std::wstring &LogFileName){
	jobs::Jobs<CHECKSUM_DWORD> jobsTodo;
	for (auto &f : Files){
		jobsTodo.Tasks.push_back(jobs::Job<CHECKSUM_DWORD>(CheckSumCalculator(std::move(const_cast<std::wstring &>(f)))));
	}
	Files.clear();

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
