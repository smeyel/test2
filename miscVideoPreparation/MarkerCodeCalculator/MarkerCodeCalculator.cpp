#include <iostream>

using namespace std;

class CodeValidator
{
private:
	//unsigned int masks[7] = { 0xC0, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x03 };	// XOR of adjacent bit pairs
	unsigned int masks[7];
	//unsigned int masks[7] = { 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };	// copy 1st 7 bits

	unsigned int get8bitXor(unsigned int value);
	unsigned int getValidationCode(unsigned int code);
	unsigned int getHammingDistance(unsigned int value1, unsigned int value2);

public:
	CodeValidator();

	unsigned int getValidatedCode(unsigned int pureCode);
	bool isValid(unsigned int fullCode);

	void showEvalResults(void);
};

CodeValidator::CodeValidator()
{
	unsigned int currentMasks[] = { 0x55, 0xAA, 0x33, 0xCC, 0xC7, 0x38, 0x0F };
	for(int i=0; i<7; i++)
	{
		masks[i] = currentMasks[i];
	}
}

unsigned int CodeValidator::get8bitXor(unsigned int value)
{
	unsigned int result = 0;
	for (int bitIdx = 7; bitIdx>=0; bitIdx--)
	{
		result ^= ((value >> bitIdx) & 0x01);
	}
	return result;
}

unsigned int CodeValidator::getValidationCode(unsigned int code)
{
	unsigned int resultCheckCode = 0;

	for(int i=6; i>=0; i--)
	{
		resultCheckCode |= get8bitXor(code & masks[i]) << i;
	}
	return resultCheckCode;
}


bool CodeValidator::isValid(unsigned int fullCode)
{
	unsigned char code = fullCode & 0x00FF;
	unsigned char check = (fullCode >> 8) & 0x00FF;
	unsigned char calculatedCheck = getValidationCode(code);
	return (check == calculatedCheck);
}

unsigned int CodeValidator::getHammingDistance(unsigned int value1, unsigned int value2)
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

unsigned int CodeValidator::getValidatedCode(unsigned int pureCode)
{
	return (getValidationCode(pureCode) << 8) | pureCode;
}


/*void printBinary(unsigned int value, int bitNumber)
{
	for(int bitIdx=bitNumber-1; bitIdx>=0; bitIdx--)
	{
		cout << ((value & (1 << bitIdx)) ? "1" : "0");
	}
	cout << endl;
}

void showResults(unsigned int checkCode, unsigned int result)
{
	cout << "Check code binary: ";
	printBinary(checkCode,7);
	cout << endl << "Full code in hex: " << hex << result << endl << "in bin: ";
	printBinary(result,15);
}*/



void CodeValidator::showEvalResults(void)
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


void main()
{
	CodeValidator validator;
	validator.showEvalResults();

	unsigned int code, result;
	cout << "Enter 8-bit base code: ";
	cin >> code;
	cout << endl << "Extended with validation code (hex): ";
	cout << hex << validator.getValidatedCode(code) << endl;

	cout << "--- Some codes for you: :)" << endl;
	for (int i=0; i<10; i++)
	{
		cout << "Base ID: " << i << ", code (hex): " << hex << validator.getValidatedCode(i) << endl;
	}

	cout << endl << "press enter..." << endl;
	cin.ignore();
}
