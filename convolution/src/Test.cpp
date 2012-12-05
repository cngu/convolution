#include <math.h>
#include <iostream>
#include "Test.h"

using namespace std;

void Test::pass(char* className, char* method)
{
	cout << className << "::" << method << " passed!" << endl;
}

void Test::fail(char* className, char* method, char* message)
{
	cout << className << "::" << method << " failed..." << endl;
	cout << " - " << message << endl;
}

bool Test::equalDoubles(double a, double b)
{
	return fabs(a-b) < 0.1;
}