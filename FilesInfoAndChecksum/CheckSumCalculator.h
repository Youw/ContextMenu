#ifndef CHECKSUMCALCULATOR_H
#define CHECKSUMCALCULATOR_H

#include<string>

class CheckSumCalculator
{
public:
	CheckSumCalculator(const std::wstring& FileName = L"");

	//return checksum or throw exception if error
	_Uint32t operator() ();
	//it may be not cross platform type. TODO: fix it later

	void SetFileName(std::wstring& FileName);

	//do not needed now
//	~CheckSumCalculator();

private:
	std::wstring m_FileName;
};
#endif //CHECKSUMCALCULATOR_H
