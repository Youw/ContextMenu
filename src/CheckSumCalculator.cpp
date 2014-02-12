#include "CheckSumCalculator.h"

#include <fstream>
#include <stdexcept>


CheckSumCalculator::CheckSumCalculator(const std::wstring& FileName):m_FileName(FileName) { 
}

CheckSumCalculator::CheckSumCalculator(std::wstring&& FileName):m_FileName(std::move(FileName)) {
}

CHECKSUM_DWORD  CheckSumCalculator::operator() () {
	CHECKSUM_DWORD sum = 0;
	char spoon = 0;

	std::fstream temp(m_FileName, std::ios::in | std::ios::binary);

	if (!temp.is_open())	throw std::runtime_error("The target file can't be opened");
	while (!temp.eof()){
		temp.read((&spoon), 1);
		sum += spoon;
	}
	temp.close();
	return (sum - spoon);
}

void CheckSumCalculator::SetFileName(const std::wstring& FileName) {
	m_FileName = FileName;
}

void CheckSumCalculator::SetFileName(std::wstring&& FileName) {
	m_FileName = FileName;
}

const std::wstring& CheckSumCalculator::GetFileName() const {
	return m_FileName;
}