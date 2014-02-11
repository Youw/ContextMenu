#ifndef MAINLOGIC_H
#define MAINLOGIC_H

#include<set>
#include<string>

//Writes specific info of Files into LogFileName
//if async==true, always return true
//else waits until log file is performed and return true on success
bool WriteInfoToLog(std::set<std::wstring> &Files, const std::wstring &LogFileName = L"./SelectedFilesInfo.log",const bool async = false);

#endif //MAINLOGIC_H