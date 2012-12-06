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