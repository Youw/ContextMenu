#ifndef CHECKSUMCALCULATOR_H
#define CHECKSUMCALCULATOR_H

#include<string>
#include<limits.h>

//need cross platform DWORD
#if (USHRT_MAX==4294967295)
#define Uint32t unsigned short
#elif (UINT_MAX==4294967295)
#define Uint32t unsigned int
#elif (ULONG_MAX==4294967295)
#define Uint32t unsigned long
#endif

#define CHECKSUM_DWORD Uint32t

//calculates a per-byte sum (a flavour of checksum) in 4 bytes variable
class CheckSumCalculator
{
public:
	CheckSumCalculator(const std::wstring& FileName = L"");
	CheckSumCalculator(std::wstring&& FileName);

	//return checksum of initialised filename or throw exception if error
	CHECKSUM_DWORD operator() ();

	void SetFileName(const std::wstring& FileName);
	void SetFileName(std::wstring&& FileName);

	const std::wstring& GetFileName() const;

	//do not needed now
//	~CheckSumCalculator();

private:
	std::wstring m_FileName;
};
#endif //CHECKSUMCALCULATOR_H
