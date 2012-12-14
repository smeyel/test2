#include <iostream>
#include "CodeValidator15bit.h"

using namespace std;

CodeValidator15bit::CodeValidator15bit()
{
	unsigned int currentMasks[] = { 0x55, 0xAA, 0x33, 0xCC, 0xC7, 0x38, 0x0F };
	for(int i=0; i<7; i++)
	{
		masks[i] = currentMasks[i];
	}
}

unsigned int CodeValidator15bit::get8bitXor(unsigned int value)
{
	unsigned int result = 0;
	for (int bitIdx = 7; bitIdx>=0; bitIdx--)
	{
		result ^= ((value >> bitIdx) & 0x01);
	}
	return result;
}

unsigned int CodeValidator15bit::getValidationCode(unsigned int code)
{
	unsigned int resultCheckCode = 0;

	for(int i=6; i>=0; i--)
	{
		resultCheckCode |= get8bitXor(code & masks[i]) << i;
	}
	return resultCheckCode;
}


bool CodeValidator15bit::isValid(unsigned int fullCode)
{
	unsigned char code = fullCode & 0x00FF;
	unsigned char check = (fullCode >> 8) & 0x00FF;
	unsigned char calculatedCheck = getValidationCode(code);
	return (check == calculatedCheck);
}

unsigned int CodeValidator15bit::getHammingDistance(unsigned int value1, unsigned int value2)
{
	int distance=0;
	for (unsigned int bit = 0x8000; bit>0; bit >>= 1)
	{
		if ((value1 & bit) ^ (value2 & bit))
		{
			distance++;
		}
	}
	return distance;
}

unsigned int CodeValidator15bit::getValidatedCode(unsigned int pureCode)
{
	return (getValidationCode(pureCode) << 8) | pureCode;
}

void CodeValidator15bit::showEvalResults(void)
{
	int sumDistValidCounters[5];
	for (int i=0; i<5; i++) 
		sumDistValidCounters[i]=0;

	for(int code = 0; code <= 0xFF; code++)
	{
		unsigned int fullCode = (getValidationCode(code) << 8) | code;

		for(int otherFullCode = 0; otherFullCode < 0x7FFF; otherFullCode++)
		{
			if (otherFullCode == fullCode)
				continue;	// Skip this one...
			if (isValid(otherFullCode))
			{
				unsigned int dist = getHammingDistance(fullCode, otherFullCode);
				if (dist<5)
				{
					sumDistValidCounters[dist]++;
				}
			}
		}
	}

	for (int i=0; i<5; i++) 
	{
		cout << "Avg number of valid codes with Hamming distance " << i << " : " << (float)(sumDistValidCounters[i])/256.0 << endl;
	}
}
