#ifndef CHECKSUMCALCULATOR_H
#define CHECKSUMCALCULATOR_H

#include<string>
#include<limits.h>
#include<cstdint>

//calculates a per-byte sum (a flavour of checksum) in 4 bytes variable
class CheckSumCalculator
{
public:
	CheckSumCalculator(const std::wstring& FileName = L"");
	CheckSumCalculator(std::wstring&& FileName);

	//return checksum of initialised filename or throw exception if error
	uint32_t operator() ();

	void SetFileName(const std::wstring& FileName);
	void SetFileName(std::wstring&& FileName);

	const std::wstring& GetFileName() const;

	//do not needed now
//	~CheckSumCalculator();

private:
	std::wstring m_FileName;
};
#endif //CHECKSUMCALCULATOR_H
