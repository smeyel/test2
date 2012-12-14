#include <iostream>
#include "CodeValidator7bit.h"

using namespace std;


unsigned int get7bitHammingDistance(unsigned int value1, unsigned int value2)
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

#define MAX_N 20
#define MAX_CODEVALUE 0x7F

unsigned int bestCodeList[MAX_N];
unsigned int bestHammingDistance = 0;

unsigned int currentCodeList[MAX_N];

void checkCodesRecursive(int n, int level, int prevLevelMinHammingDistance)
{
	// Iterate along the valid values on level "level"
	// For each of them, calculate minimal hamming code with previous levels,
	//	if min. hamming code is higher than bestHammingDistance, call recursively for next level
	if (level <= n)
	{
		// Go along all possible codes
		for (unsigned int currentCode = 0; currentCode <= MAX_CODEVALUE; currentCode++)
		{
			if (level == 0)
			{
				cout << "L0 value: " << currentCode << endl;
			}


			//cout << "Checking value " << currentCode << " on level " << level << endl;
			currentCodeList[level] = currentCode;

			int minHammingDist = MAX_CODEVALUE;	// Surely maximal
			// Check min. hamming distance with all previous codes
			for (int checkedLevel = 0; checkedLevel < level; checkedLevel++)
			{
				int hdist = get7bitHammingDistance(currentCode,currentCodeList[checkedLevel]);
				if (hdist<minHammingDist)
				{
					minHammingDist = hdist;
				}
			}
			//cout << "  Min H-dist from here: " << minHammingDist;
			// h-dist among codes of previous levels may be higher then current min:
			if (minHammingDist>prevLevelMinHammingDistance)
			{
				minHammingDist=prevLevelMinHammingDistance;
				//cout << " taking prev: " << minHammingDist;
			}
			//cout << endl;

			// If the h-dist of this value is higher than the best until now (bestHammingDistance),
			// (that is, we can still improve it), go on for the next level
			if (minHammingDist > bestHammingDistance)
			{
				//cout << "  Still better than best (" << bestHammingDistance << "), going to next level." << endl;
				checkCodesRecursive(n,level+1,minHammingDist);
			}
		}
	}
	else
	{
		// No more levels.
		cout << "Checking current solution: ";
		for(int i=0; i<n; i++)
			cout << currentCodeList[i] << " ";
		cout << ", min h-dist: " << prevLevelMinHammingDistance << " -> ";

		// Store if result is better.
		if (prevLevelMinHammingDistance > bestHammingDistance)
		{
			// Copy (until now) best solution
			cout << "New best!";
			for(int i=0; i<n; i++)
			{
				bestCodeList[i]=currentCodeList[i];
				bestHammingDistance = prevLevelMinHammingDistance;
			}
		}
		cout << endl;
	}
}


void GetN7bitCodesBruteForce(int n)
{
	checkCodesRecursive(n, 0, MAX_CODEVALUE);
}

void main()
{
	int n=5;
	GetN7bitCodesBruteForce(n);
	cout << "ready (hex): ";
	for(int i=0; i<n; i++)
	{
		cout << hex << bestCodeList[i] << " ";
	}
	cout << " H-dist=" << bestHammingDistance << endl;
	int dummy;
	cin >> dummy;



/*	CodeValidator7bit validator;
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
	cin.ignore(); */
}
