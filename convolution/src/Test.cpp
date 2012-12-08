#include <math.h>
#include <iostream>
#include <string>
#include "Test.h"

using namespace std;

void Test::pass(string className, string method)
{
	cout << className << "::" << method << " passed!" << endl;
}

void Test::fail(string className, string method)
{
	cout << className << "::" << method << " failed..." << endl;
}

bool Test::equalShorts(short a, short b)
{
	if (a == b || a+1 == b || a-1 == b)
		return true;
	return false;
}

bool Test::equalDoubles(double a, double b)
{
	return fabs(a-b) < 0.1;
}

void Test::printArray(short* array, int len) 
{
	int i = 0;
	for (; i < len-1; i++) 
		cout << array[i] << ", ";
	cout << array[i] << endl;
}

void Test::printArray(double* array, int len) 
{
	int i = 0;
	for (; i < len-1; i++) 
		cout << array[i] << ", ";
	cout << array[i] << endl;
}

bool Test::equalArrays(short* a, short* b, int len)
{
	for (int i = 0; i < len; i++)
		if (! equalShorts(a[i], b[i]))
			return false;

	return true;
}

bool Test::equalArrays(double* a, double* b, int len)
{
	for (int i = 0; i < len; i++)
		if (! equalDoubles(a[i], b[i]))
			return false;

	return true;
}

void Test::clearArray(double* array, int len) 
{
	for (int i = 0; i < len; i++)
		array[i] = 0.0;
}

void Test::clearArray(short* array, int len)
{
	for (int i = 0; i < len; i++)
		array[i] = 0;
}
