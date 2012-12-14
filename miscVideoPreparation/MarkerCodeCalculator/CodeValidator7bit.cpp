#include <iostream>
#include "CodeValidator7bit.h"

using namespace std;

CodeValidator7bit::CodeValidator7bit()
{
	unsigned int currentMasks[] = { 0x0F, 0x05, 0x0A };
	for(int i=0; i<7; i++)
	{
		masks[i] = currentMasks[i];
	}
}

unsigned int CodeValidator7bit::get4bitXor(unsigned int value)
{
	unsigned int result = 0;
	for (int bitIdx = 3; bitIdx>=0; bitIdx--)
	{
		result ^= ((value >> bitIdx) & 0x01);
	}
	return result;
}

unsigned int CodeValidator7bit::getValidationCode(unsigned int code)
{
	unsigned int resultCheckCode = 0;

	for(int i=2; i>=0; i--)
	{
		resultCheckCode |= get4bitXor(code & masks[i]) << i;
	}
	return resultCheckCode;
}


bool CodeValidator7bit::isValid(unsigned int fullCode)
{
	unsigned char code = fullCode & 0x000F;	// 4 bits
	unsigned char check = (fullCode >> 4) & 0x0007;	// 3 bits
	unsigned char calculatedCheck = getValidationCode(code);
	return (check == calculatedCheck);
}

unsigned int CodeValidator7bit::getHammingDistance(unsigned int value1, unsigned int value2)
{
	int distance=0;
	for (unsigned int bit = 0x40; bit>0; bit >>= 1)
	{
		if ((value1 & bit) ^ (value2 & bit))
		{
			distance++;
		}
	}
	return distance;
}

unsigned int CodeValidator7bit::getValidatedCode(unsigned int pureCode)
{
	return (getValidationCode(pureCode) << 4) | pureCode;
}

void CodeValidator7bit::showEvalResults(void)
{
	int sumDistValidCounters[5];
	for (int i=0; i<5; i++) 
		sumDistValidCounters[i]=0;

	for(int code = 0; code <= 0x0F; code++)
	{
		cout << "Code:" << code;
		unsigned int fullCode = (getValidationCode(code) << 4) | code;

		for(int otherFullCode = 0; otherFullCode < 0x7F; otherFullCode++)
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
				if (dist <= 2)
				{
					cout << "+";
				}
			}
		}
		cout << endl;
	}

	for (int i=1; i<5; i++) 
	{
		cout << "Avg number of valid codes with Hamming distance " << i << " : " << (float)(sumDistValidCounters[i])/0x0F << endl;
	}
}
