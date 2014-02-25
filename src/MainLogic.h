#ifndef MAINLOGIC_H
#define MAINLOGIC_H

#include "JobsExecuting.h"
#include "CheckSumCalculator.h"
#include <set>
#include <string>
#include <thread>
#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>
#include <chrono>
#include <locale>
#include <ctime>

template<typename T>
//Writes specific info of Files into LogFileName
//if async==true, always return true
//else waits until log file is performed and return true on success
//T must be STL container or array of std::wstring
bool WriteInfoToLog(T&& Files, const std::wstring &LogFileName = L"./SelectedFilesInfo.log", const bool async = false);

template<typename T>
static bool m_WriteInfoToLog(T&& Files, const std::wstring &LogFileName){
	using std::endl;
	std::wofstream LogFile;
	LogFile.imbue(std::locale(""));

	size_t filesCount = Files.size();

	std::vector<jobs::Job<uint32_t>> jobsTodo;
	for (auto &f : Files){
		jobsTodo.push_back(jobs::Job<uint32_t>(CheckSumCalculator(std::move(const_cast<std::wstring &>(f)))));
	}
	Files.clear();
	//start of the log
	{
		LogFile.open(LogFileName, std::ios::app);
		if (LogFile) {
			auto&& now = boost::posix_time::second_clock::local_time();
			LogFile << endl << "  <<<< Log created at: " << now << " >>>>" << endl << "Selected " << filesCount << " file(s). Info:" << endl << endl;
			LogFile.close();
		}
		else {
			return false;
		}
	}

	jobs::JobsExecuter<uint32_t> myJobs(std::move(jobsTodo));

	size_t jobsCount = myJobs.JobsCount();

	for (size_t i = 0; i < jobsCount; i++) {
		using boost::filesystem::path;
		using boost::filesystem::is_regular_file;
		using boost::filesystem::is_directory;
		using boost::filesystem::file_size;
		using boost::filesystem::last_write_time;
		using boost::posix_time::from_time_t;
		using boost::date_time::c_local_adjustor;
		using boost::posix_time::ptime;
		static const unsigned long s_byte = 0x400;					// 1024
		static const unsigned long s_Kbyte = 0x100000;				//	s_byte*1024;
		static const unsigned long s_Mbyte = 0x40000000;			//	s_Kbyte * 1024;
		static const unsigned long long s_Gbyte = 0x10000000000;	//	s_Mbyte*1024
//		static const unsigned long s_Tbyte = s_Gbyte * 1024;		//  O_0, mb some oracle db? =)

		LogFile.open(LogFileName, std::ios::app);
		uint32_t checksum = myJobs[i];
		path PathName(myJobs.GetJob(i).Task.target<CheckSumCalculator>()->GetFileName());
		if (is_regular_file(PathName)) {
			//i haven't discover fine cross platform solution for getting
			//file creation time, so let it be modification time for now
			LogFile << "Modified: " << c_local_adjustor<ptime>::utc_to_local(from_time_t(last_write_time(PathName)));
			
			LogFile.imbue(std::locale("C"));
			LogFile << " || Per-byte sum: 0x" << std::setfill(L'0') << std::hex << std::noshowbase << std::setw(8) << checksum << std::dec;
			LogFile.imbue(std::locale(""));

			auto&& fileSize = file_size(PathName);
			LogFile << " || Size:" << std::fixed << std::setprecision(2) << std::setw(8) << std::setfill(L' ');
			if (fileSize<s_byte)
				LogFile << fileSize << "  b";
			else if (fileSize < s_Kbyte)
				LogFile << double(fileSize) / s_byte << " Kb";
			else if (fileSize < s_Mbyte)
				LogFile << double(fileSize) / s_Kbyte << " Mb";
			else if (fileSize < s_Gbyte)
				LogFile << double(fileSize) / s_Mbyte << " Gb";
			else //if (fileSize < s_Tbyte)
				LogFile << double(fileSize) / s_Gbyte << " Tb";
			LogFile << " || File name: " << PathName.filename() << endl;
			LogFile.close();
		}
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
bool WriteInfoToLog(T&& Files, const std::wstring &LogFileName, const bool async){
	if (async) {
		//i think it has a bag
		//TODO: fix it later
		std::thread working_thread(m_WriteInfoToLog<T>, std::move(Files), std::move(LogFileName));
		working_thread.detach();
	}
	else {
		return m_WriteInfoToLog<T>(Files, LogFileName);
	}
	return true;
}


#endif //MAINLOGIC_H