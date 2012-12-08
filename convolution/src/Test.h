#ifndef TEST_H
#define TEST_H

#include <string>

class Test 
{
public:
	static void pass(std::string className, std::string method);
	static void fail(std::string className, std::string method);

	static bool equalShorts(short a, short b);
	static bool equalDoubles(double a, double b);
	static bool equalArrays(short* a, short* b, int len);
	static bool equalArrays(double* a, double* b, int len);

	static void printArray(double* array, int len);
	static void printArray(short* array, int len);

	static void clearArray(double* array, int len);
	static void clearArray(short* array, int len);
};

#endif