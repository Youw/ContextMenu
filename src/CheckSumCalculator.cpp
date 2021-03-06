#include "CheckSumCalculator.h"

#include <iostream>
#include <fstream>
#include <stdexcept>


CheckSumCalculator::CheckSumCalculator(const std::wstring& FileName):m_FileName(FileName) { 
}

CheckSumCalculator::CheckSumCalculator(std::wstring&& FileName):m_FileName(std::move(FileName)) {
}

uint32_t  CheckSumCalculator::operator() () {
	uint32_t sum = 0;
	char spoon[4096];

	std::ifstream temp(m_FileName, std::ios::binary);
	if (!temp.is_open())	throw std::runtime_error("The target file can't be opened");
	
	temp.read(spoon, 4096);
	while (!temp.eof()) {
		for (auto&i:spoon)
			sum += uint32_t(i);
		temp.read(spoon, 4096);
	}
	if (temp.bad()) throw std::runtime_error("Error occured while reading file");
	long long readed = temp.gcount();
	for (int i = 0; i < readed; i++) {
		sum += uint32_t(spoon[i]);
	}
	temp.close();
	return (sum);
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