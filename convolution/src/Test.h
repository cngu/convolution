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
};

#endif