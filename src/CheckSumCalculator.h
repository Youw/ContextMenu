#ifndef CHECKSUMCALCULATOR_H
#define CHECKSUMCALCULATOR_H

#include<string>
#include<limits.h>

#if (USHRT_MAX==4294967295)
#define Uint32t unsigned short
#elif (UINT_MAX==4294967295)
#define Uint32t unsigned int
#elif (ULONG_MAX==4294967295)
#define Uint32t unsigned long
#endif



class CheckSumCalculator
{
public:
	CheckSumCalculator(const std::wstring& FileName = L"");

	//return checksum or throw exception if error
	Uint32t operator() ();
	//it may be not cross platform type. TODO: fix it later

	void SetFileName(std::wstring& FileName);

	//do not needed now
//	~CheckSumCalculator();

private:
	std::wstring m_FileName;
};
#endif //CHECKSUMCALCULATOR_H
