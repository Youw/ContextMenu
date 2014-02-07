#include <fstream>
#include "CheckSumCalculator.h"

CheckSumCalculator::CheckSumCalculator(const std::wstring& FileName):m_FileName(FileName)
{ }

Uint32t  CheckSumCalculator::operator() (){
	Uint32t sum = 0;
	char spoon = 0;

	std::fstream temp(m_FileName,std::ios::in | std::ios::binary);

	if(!temp.is_open())	throw std::runtime_error("The target file can't be opened");
	while(!temp.eof()){
		temp.read((&spoon),1);
		sum+=spoon;
	}
	
  return (sum - spoon);
}

void CheckSumCalculator::SetFileName(std::wstring& FileName){
	m_FileName = FileName;
}