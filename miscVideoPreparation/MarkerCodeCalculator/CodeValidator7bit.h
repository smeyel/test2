#ifndef __CODEVALIDATOR7BIT_H_
#define __CODEVALIDATOR7BIT_H_

class CodeValidator7bit
{
private:
	//unsigned int masks[7] = { 0xC0, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x03 };	// XOR of adjacent bit pairs
	unsigned int masks[3];
	//unsigned int masks[7] = { 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };	// copy 1st 7 bits

	unsigned int get4bitXor(unsigned int value);
	unsigned int getValidationCode(unsigned int code);
	unsigned int getHammingDistance(unsigned int value1, unsigned int value2);

public:
	CodeValidator7bit();

	unsigned int getValidatedCode(unsigned int pureCode);
	bool isValid(unsigned int fullCode);

	void showEvalResults(void);
};

#endif
